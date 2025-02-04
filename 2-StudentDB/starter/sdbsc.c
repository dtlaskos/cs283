#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

// Database include files
#include "db.h"
#include "sdbsc.h"

/*
 *  open_db
 *      dbFile:  name of the database file
 *      should_truncate:  indicates if opening the file also empties it
 *
 *  returns:  File descriptor on success, or ERR_DB_FILE on failure
 *
 *  console:  Does not produce any console I/O on success
 *            M_ERR_DB_OPEN on error
 *
 */
int open_db(char *dbFile, bool should_truncate) {
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    int flags = O_RDWR | O_CREAT;

    if (should_truncate)
        flags |= O_TRUNC;

    int fd = open(dbFile, flags, mode);

    if (fd == -1) {
        printf(M_ERR_DB_OPEN);
        return ERR_DB_FILE;
    }

    return fd;
}


/*
 *  get_student
 *      fd:  linux file descriptor
 *      id:  the student id we are looking for
 *      *s:  a pointer where the located (if found) student data will be copied
 *
 *  returns:  NO_ERROR       student located and copied into *s
 *            ERR_DB_FILE    database file I/O issue
 *            SRCH_NOT_FOUND student was not located in the database
 */
int get_student(int fd, int id, student_t *s) {
    off_t offset = id * sizeof(student_t);
    if (lseek(fd, offset, SEEK_SET) == -1) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }

    ssize_t bytes_read = read(fd, s, sizeof(student_t));
    if (bytes_read == -1) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    } else if (bytes_read != sizeof(student_t)) {
        s->id = 0;
        return SRCH_NOT_FOUND;
    }

    if (s->id == 0) {
        return SRCH_NOT_FOUND;
    }

    return NO_ERROR;
}

/*
 *  add_student
 *      fd:     linux file descriptor
 *      id:     student id
 *      fname:  student first name
 *      lname:  student last name
 *      gpa:    GPA as an integer
 *
 *  returns:  NO_ERROR       student added to database
 *            ERR_DB_FILE    database file I/O issue
 *            ERR_DB_OP      student already exists
 */
int add_student(int fd, int id, char *fname, char *lname, int gpa) {
    student_t student;
    student_t new_student = {0};

    if (validate_range(id, gpa) != NO_ERROR) {
        printf(M_ERR_STD_RNG);
        return ERR_DB_OP;
    }

    int rc = get_student(fd, id, &student);
    if (rc == NO_ERROR) {
        printf(M_ERR_DB_ADD_DUP, id);
        return ERR_DB_OP;
    } else if (rc != SRCH_NOT_FOUND) {
        return rc;
    }

    new_student.id = id;
    strncpy(new_student.fname, fname, sizeof(new_student.fname) - 1);
    strncpy(new_student.lname, lname, sizeof(new_student.lname) - 1);
    new_student.gpa = gpa;

    off_t offset = id * sizeof(student_t);
    if (lseek(fd, offset, SEEK_SET) == -1) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }

    if (write(fd, &new_student, sizeof(student_t)) != sizeof(student_t)) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }

    printf(M_STD_ADDED, id);
    return NO_ERROR;
}

/*
 *  del_student
 *      fd:     linux file descriptor
 *      id:     student id to be deleted
 *
 *  returns:  NO_ERROR       student deleted from database
 *            ERR_DB_FILE    database file I/O issue
 *            ERR_DB_OP      student not in database
 */
int del_student(int fd, int id) {
    student_t student;

    int rc = get_student(fd, id, &student);
    if (rc != NO_ERROR) {
        if (rc == SRCH_NOT_FOUND) {
            printf(M_STD_NOT_FND_MSG, id);
        }
        return ERR_DB_OP;
    }

    off_t offset = id * sizeof(student_t);
    if (lseek(fd, offset, SEEK_SET) == -1) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }

    if (write(fd, &EMPTY_STUDENT_RECORD, sizeof(student_t)) != sizeof(student_t)) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }

    printf(M_STD_DEL_MSG, id);
    return NO_ERROR;
}

/*
 *  count_db_records
 *      fd:     linux file descriptor
 *
 *  returns:  <number>       number of records in db on success
 *            ERR_DB_FILE    database file I/O issue
 */
int count_db_records(int fd) {
    student_t student;
    int count = 0;

    if (lseek(fd, 0, SEEK_SET) == -1) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }

    while (read(fd, &student, sizeof(student_t)) == sizeof(student_t)) {
        if (student.id != 0) {
            count++;
        }
    }

    if (count == 0) {
        printf(M_DB_EMPTY);
    } else {
        printf(M_DB_RECORD_CNT, count);
    }

    return count;
}

/*
 *  print_db
 *      fd:     linux file descriptor
 *
 *  returns:  NO_ERROR       on success
 *            ERR_DB_FILE    database file I/O issue
 */
int print_db(int fd) {
    student_t student;
    bool header_printed = false;

    if (lseek(fd, 0, SEEK_SET) == -1) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }

    while (read(fd, &student, sizeof(student_t)) == sizeof(student_t)) {
        if (student.id != 0) {
            if (!header_printed) {
                printf(STUDENT_PRINT_HDR_STRING, "ID", "FIRST_NAME", "LAST_NAME", "GPA");
                header_printed = true;
            }
            float gpa = student.gpa / 100.0;
            printf(STUDENT_PRINT_FMT_STRING, student.id, student.fname, student.lname, gpa);
        }
    }

    if (!header_printed) {
        printf(M_DB_EMPTY);
    }

    return NO_ERROR;
}

/*
 *  print_student
 *      *s:   a pointer to a student_t structure
 */
void print_student(student_t *s) {
    if (s == NULL || s->id == 0) {
        printf(M_ERR_STD_PRINT);
        return;
    }

    float gpa = s->gpa / 100.0;
    printf(STUDENT_PRINT_HDR_STRING, "ID", "FIRST NAME", "LAST NAME", "GPA");
    printf(STUDENT_PRINT_FMT_STRING, s->id, s->fname, s->lname, gpa);
}

/*
 *  compress_db
 *      fd:     linux file descriptor
 *
 *  returns:  <number>       fd of the compressed database file
 *            ERR_DB_FILE    database file I/O issue
 */
int compress_db(int fd) {
    int tmp_fd;
    student_t student;

    tmp_fd = open(TMP_DB_FILE, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (tmp_fd == -1) {
        printf(M_ERR_DB_OPEN);
        return ERR_DB_FILE;
    }

    if (lseek(fd, 0, SEEK_SET) == -1) {
        printf(M_ERR_DB_READ);
        close(tmp_fd);
        return ERR_DB_FILE;
    }

    ssize_t bytes_read;
    while ((bytes_read = read(fd, &student, sizeof(student_t))) > 0) {
        if (bytes_read != sizeof(student_t)) {
            printf(M_ERR_DB_READ);
            close(tmp_fd);
            return ERR_DB_FILE;
        }
        if (student.id != 0) {
            if (write(tmp_fd, &student, sizeof(student_t)) != sizeof(student_t)) {
                printf(M_ERR_DB_WRITE);
                close(tmp_fd);
                return ERR_DB_FILE;
            }
        }
    }

    if (bytes_read == -1) {
        printf(M_ERR_DB_READ);
        close(tmp_fd);
        return ERR_DB_FILE;
    }

    close(fd);
    close(tmp_fd);

    if (rename(TMP_DB_FILE, DB_FILE) == -1) {
        printf(M_ERR_DB_CREATE);
        return ERR_DB_FILE;
    }

    fd = open_db(DB_FILE, false);
    if (fd == -1) {
        printf(M_ERR_DB_OPEN);
        return ERR_DB_FILE;
    }

    printf(M_DB_COMPRESSED_OK);
    return fd;
}

/*
 *  validate_range
 *      id:  proposed student id
 *      gpa: proposed gpa
 *
 *  This function validates that the id and gpa are in the allowable ranges
 *  as per the specifications.  It checks if the values are within the
 *  inclusive range using constents in db.h
 *
 *  returns:    NO_ERROR       on success, both ID and GPA are in range
 *              EXIT_FAIL_ARGS if either ID or GPA is out of range
 *
 *  console:  This function does not produce any output
 *
 */
int validate_range(int id, int gpa) {
    if (id < MIN_STD_ID || id > MAX_STD_ID)
        return EXIT_FAIL_ARGS;

    if (gpa < MIN_STD_GPA || gpa > MAX_STD_GPA)
        return EXIT_FAIL_ARGS;

    return NO_ERROR;
}

/*
 *  usage
 *      exename:  the name of the executable from argv[0]
 *
 *  Prints this programs expected usage
 *
 *  returns:    nothing, this is a void function
 *
 *  console:  This function prints the usage information
 *
 */
void usage(char *exename) {
    printf("usage: %s -[h|a|c|d|f|p|z] options.  Where:\n", exename);
    printf("\t-h:  prints help\n");
    printf("\t-a id first_name last_name gpa(as 3 digit int):  adds a student\n");
    printf("\t-c:  counts the records in the database\n");
    printf("\t-d id:  deletes a student\n");
    printf("\t-f id:  finds and prints a student in the database\n");
    printf("\t-p:  prints all records in the student database\n");
    printf("\t-x:  compress the database file [EXTRA CREDIT]\n");
    printf("\t-z:  zero db file (remove all records)\n");
}


// Welcome to main()
int main(int argc, char *argv[])
{
    char opt;      // user selected option
    int fd;        // file descriptor of database files
    int rc;        // return code from various operations
    int exit_code; // exit code to shell
    int id;        // userid from argv[2]
    int gpa;       // gpa from argv[5]

    // space for a student structure which we will get back from
    // some of the functions we will be writing such as get_student(),
    // and print_student().
    student_t student = {0};

    // This function must have at least one arg, and the arg must start
    // with a dash
    if ((argc < 2) || (*argv[1] != '-'))
    {
        usage(argv[0]);
        exit(1);
    }

    // The option is the first character after the dash for example
    //-h -a -c -d -f -p -x -z
    opt = (char)*(argv[1] + 1); // get the option flag

    // handle the help flag and then exit normally
    if (opt == 'h')
    {
        usage(argv[0]);
        exit(EXIT_OK);
    }

    // now lets open the file and continue if there is no error
    // note we are not truncating the file using the second
    // parameter
    fd = open_db(DB_FILE, false);
    if (fd < 0) {
        exit(EXIT_FAIL_DB);
    }

    // set rc to the return code of the operation to ensure the program
    // use that to determine the proper exit_code.  Look at the header
    // sdbsc.h for expected values.

    exit_code = EXIT_OK;
    switch (opt)
    {
    case 'a':
        //   arv[0] arv[1]  arv[2]      arv[3]    arv[4]  arv[5]
        // prog_name     -a      id  first_name last_name     gpa
        //-------------------------------------------------------
        // example:  prog_name -a 1 John Doe 341
        if (argc != 6)
        {
            usage(argv[0]);
            exit_code = EXIT_FAIL_ARGS;
            break;
        }

        // convert id and gpa to ints from argv.  For this assignment assume
        // they are valid numbers
        id = atoi(argv[2]);
        gpa = atoi(argv[5]);

        exit_code = validate_range(id, gpa);
        if (exit_code == EXIT_FAIL_ARGS)
        {
            printf(M_ERR_STD_RNG);
            break;
        }

        rc = add_student(fd, id, argv[3], argv[4], gpa);
        if (rc < 0)
            exit_code = EXIT_FAIL_DB;

        break;

    case 'c':
        //    arv[0] arv[1]
        // prog_name     -c
        //-----------------
        // example:  prog_name -c
        rc = count_db_records(fd);
        if (rc < 0)
            exit_code = EXIT_FAIL_DB;
        break;

    case 'd':
        //   arv[0]  arv[1]  arv[2]
        // prog_name     -d      id
        //-------------------------
        // example:  prog_name -d 100
        if (argc != 3)
        {
            usage(argv[0]);
            exit_code = EXIT_FAIL_ARGS;
            break;
        }
        id = atoi(argv[2]);
        rc = del_student(fd, id);
        if (rc < 0)
            exit_code = EXIT_FAIL_DB;

        break;

    case 'f':
        //    arv[0] arv[1]  arv[2]
        // prog_name     -f      id
        //-------------------------
        // example:  prog_name -f 100
        if (argc != 3)
        {
            usage(argv[0]);
            exit_code = EXIT_FAIL_ARGS;
            break;
        }
        id = atoi(argv[2]);
        rc = get_student(fd, id, &student);

        switch (rc)
        {
        case NO_ERROR:
            print_student(&student);
            break;
        case SRCH_NOT_FOUND:
            printf(M_STD_NOT_FND_MSG, id);
            exit_code = EXIT_FAIL_DB;
            break;
        default:
            printf(M_ERR_DB_READ);
            exit_code = EXIT_FAIL_DB;
            break;
        }
        break;

    case 'p':
        //    arv[0] arv[1]
        // prog_name     -p
        //-----------------
        // example:  prog_name -p
        rc = print_db(fd);
        if (rc < 0)
            exit_code = EXIT_FAIL_DB;
        break;

    case 'x':
        //    arv[0] arv[1]
        // prog_name     -x
        //-----------------
        // example:  prog_name -x

        // remember compress_db returns a fd of the compressed database.
        // we close it after this switch statement
        fd = compress_db(fd);
        if (fd < 0)
            exit_code = EXIT_FAIL_DB;
        break;

    case 'z':
        //    arv[0] arv[1]
        // prog_name     -x
        //-----------------
        // example:  prog_name -x
        // HINT:  close the db file, we already have fd
        //       and reopen db indicating truncate=true
        close(fd);
        fd = open_db(DB_FILE, true);
        if (fd < 0)
        {
            exit_code = EXIT_FAIL_DB;
            break;
        }
        printf(M_DB_ZERO_OK);
        exit_code = EXIT_OK;
        break;
    default:
        usage(argv[0]);
        exit_code = EXIT_FAIL_ARGS;
    }

    // dont forget to close the file before exiting, and setting the
    // proper exit code - see the header file for expected values
    close(fd);
    exit(exit_code);
}
