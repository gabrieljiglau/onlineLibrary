#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sqlite3.h>
#include <sodium.h>
#include <time.h>

#define SALT_SIZE crypto_pwhash_SALTBYTES

#define SMALL_BUFFER_SIZE 128
#define BUFFER_SIZE 1024
#define PORT_NUMBER 34324

#define DATABASE_PATH "LibraryManagerDB"

#define PASSWORD_LENGTH 64
#define HASH_ERROR "hash function error"
#define UNEXPECTED_ERROR -1

#define MAX_CLIENTS 5
#define FD_SIZE 6
#define OCCUPIED_INDEX 35

#define TYPE_COMMAND "Enter a command to run ..\n"
#define NO_PRIVILEGES "No privileges.. \nYou must login first\n"
#define RECOGNIZE_SHUDOWN "SHUTDOWN"
#define RECOGNIZE_DOWNLOAD "DOWNLOAD : "
#define RECOGNIZE_QUIT "QUIT"

#define LOGIN_PREFIX "LOGIN : "
#define INCORRECT_USE_OF_LOGIN "Incorrect use of login ! \nThe syntax should be 'LOGIN : yourUsername, PASSWORD : yourPassword"
#define LOGOUT "LOGOUT"
#define MACRO_FILE_TRANSFER_COMPLETE "File transfer completed"

#define MACRO_ERROR_SQL "Unexpected error sqlite\n"
#define INCORRECT_USE_OF_DOWNLOAD "Incorrect use of download! \nThe syntax shold be 'DOWNLOAD : bookTitle, authorName"
#define INCORRECT_PASSWORD "Incorrect password\n"
#define ERROR_PASSWORD_VERIFICATION "Unexpected error in password verification\n"
#define NEW_USER_MESSAGE "New user, inserting into db\n"
#define ERROR_USER_VERIFICATION "Unexpected error in user verification\n"
#define MACRO_GET_RATING "SELECT SUM(rating), COUNT(*) FROM userHistory WHERE action = 'rate' AND book_id = ?;"

#define MACRO_RATING_OUT_OF_BOUNDS "The rating should be between 0 and 5(inclusive)!"
#define GET_ALL_AUTHORS "Get authors"
#define GET_BEST_BOOK_BY_RATING "Get books by rating"
#define GET_SUBGENRES "Get subgenres"
#define GET_GENRES "Get genres"
#define GET_ALL_BOOKS "Get all books"
#define GET_ALL_GENRES_AND_SUBGENRES "Get genres and subgenres"
#define GET_BOOKS_BY_AUTHOR_PREFIX "Get books by author : "
#define GET_RECENTLY_ADDED_BOOKS "Get newest added books : "

#define MACRO_GET_MY_RECOMMENDATIONS "Get my recommendations"
#define MACRO_GET_MY_DOWNLOADS "Get my downloads"

#define MACRO_USER_MOST_DOWNLOADED_GENRE "SELECT sg.genre_id,sg.subgenre_id ,COUNT(*) AS download_count FROM userHistory uh JOIN books b ON uh.book_id = b.book_id JOIN subgenres sg ON b.subgenre_id = sg.subgenre_id WHERE uh.user_id = ? AND uh.action = 'download' GROUP BY sg.genre_id ORDER BY download_count DESC LIMIT 1;"
#define MACRO_GET_BOOKS_BY_GENRE_AND_SUBGENRE "SELECT b.title FROM books b JOIN subgenres sg ON b.subgenre_id = sg.subgenre_id WHERE sg.genre_id = ? AND b.subgenre_id = ? LIMIT 4;"
#define MACRO_DOWNLOADED_BOOKS_FOR_USER "SELECT b.title, a.name FROM userHistory AS uh JOIN books AS b ON uh.book_id = b.book_id JOIN authors AS a ON b.author_id = a.author_id WHERE uh.user_id = ? AND uh.action = 'download';"

#define MACRO_INSERT_IN_USER_HISTORY_FOR_RATING "INSERT INTO userHistory (book_id, user_id, action, timestamp) VALUES (?, ?, ?, ?);"
#define ILLEGAL_USE_OF_LOGIN "Cannot login if you are already logged in"
#define LOGOUT_RESPONSE "Logout successful"
#define SUCCESSFUL_AUTHENTICATION "Successful authentication !"
#define QUIT_RESPONSE "You want to quit the connection.."

#define MACRO_ERROR_SQL "Unexpected error sqlite\n"
#define MACRO_SUCCESSFUL_RATING "Successful rating!"
#define MACRO_ONLY_ONE_RATING_ALLOWED "You can only rate a book once!"
#define MACRO_MUST_DOWNLOAD_BOOK_BEFORE_RATING "User must download the book before rating \n"

#define MACRO_DID_DOWNLOAD "SELECT COUNT(*) FROM userHistory WHERE user_id = ? AND book_id = ? AND action = 'download';"
#define MACRO_DID_RATE "SELECT COUNT(*) FROM userHistory WHERE user_id = ? AND book_id = ? AND action = 'rate';"
#define MACRO_GET_BOOKID "SELECT book_id FROM books WHERE title = ?;"
#define MACRO_GET_USERID "SELECT user_id FROM users WHERE username = ?;"
#define UNKNOWN_COMMAND "Unknown command !"
#define ERROR_SALT_RETRIEVAL "Salt retrieval went wrong !"

#define GET_MOST_POPULAR_GENRES "Get most popular genres"
#define GET_MOST_POPULAR_AUTHORS "Get most popular authors"
#define MACRO_ALL_GENRES "SELECT name FROM genres;"
#define MACRO_ALL_SUBGENRES "SELECT name FROM subgenres;";
#define MACRO_ALL_AUTHORS "SELECT * FROM authors;"
#define MACRO_SELECT_USERNAME "SELECT username FROM users WHERE username = ?;"
#define MACRO_INSERT_INTO_USER_HISTORY_FOR_DOWNLOAD "INSERT INTO userHistory (user_id, book_id, action, timestamp) VALUES (?, ?, ?, ?);"
#define MACRO_GET_FILE_PATH_FROM_TITLE "SELECT file_path FROM books WHERE title = ?;"

#define MACRO_NO_INFORMATION_OR_ERROR "No information or unexpected error"
#define MACRO_GENRES_WITH_SUBGENRES "SELECT DISTINCT genres.name AS genre, subgenres.name AS subgenre FROM genres LEFT JOIN subgenres ON genres.genre_id = subgenres.genre_id;"
#define MACRO_ALL_BOOKS_BY_AUTHOR "SELECT * FROM books WHERE author_id = (SELECT author_id FROM authors WHERE name = ?);"
#define MACRO_RECOMMENDATION "SELECT DISTINCT b.title FROM books b JOIN userHistory uh1 ON b.book_id = uh1.book_id JOIN userHistory uh2 ON b.author_id = uh2.author_id JOIN userHistory uh3 ON b.genre_id = uh3.genre_id WHERE uh1.user_id = ? AND uh2.user_id = ? AND uh3.user_id = ? AND (COUNT(DISTINCT uh1.book_id) > 2 OR (COUNT(DISTINCT uh2.book_id) > (1/3 * COUNT(DISTINCT uh1.book_id))) OR (COUNT(DISTINCT uh3.book_id) > (1/3 * COUNT(DISTINCT uh1.book_id))));"
#define MACRO_MOST_POPULAR_SUBGENRES "SELECT sg.*, COUNT(uh.book_id) as search_count FROM subgenres sg JOIN books b ON sg.subgenre_id = b.subgenre_id JOIN userHistory uh ON b.book_id = uh.book_id WHERE uh.action = ? GROUP BY sg.subgenre_id ORDER BY search_count DESC;"
#define MACRO_MOST_POPULAR_AUTHORS "SELECT a.name AS author_name, g.name AS genre_name, COUNT(uh.book_id) AS search_count FROM authors a JOIN books  ON a.author_id = books.author_id JOIN userHistory uh ON books.book_id = uh.book_id JOIN subgenres sg ON books.subgenre_id = sg.subgenre_id JOIN  genres g ON sg.genre_id = g.genre_id  WHERE uh.action = ? GROUP BY a.author_id ORDER BY search_count DESC;"
#define MACRO_MOST_POPULAR_GENRES "SELECT g.*, COUNT(uh.book_id) as download_count FROM genres g JOIN subgenres sg ON g.genre_id = sg.genre_id JOIN books b ON sg.subgenre_id = b.subgenre_id JOIN userHistory uh ON b.book_id = uh.book_id WHERE uh.action = ? GROUP BY g.genre_id ORDER BY download_count DESC;"
#define MACRO_NO_INFORMATION_IN_DATABASE "No relevant information in database"

#define MACRO_MOST_POPULAR_BOOKS_BY_SEARCH "SELECT b.title, b.description, COUNT(uh.book_id) as search_count FROM books b JOIN userHistory uh ON b.book_id = uh.book_id WHERE uh.action = ? GROUP BY b.book_id ORDER BY search_count DESC;"
#define MACRO_RECENTLY_ADDED_BOOKS "SELECT book_id, title FROM books ORDER BY book_id DESC LIMIT ?;"
#define MACRO_ALL_BOOKS "SELECT * FROM books;"
#define MACRO_BEST_BOOKS_BY_RATING "SELECT * FROM books ORDER BY rating DESC;"
#define MACRO_SELECT_FROM_CLIENTS_LOGIN "SELECT password FROM users WHERE username = ?;"
#define MACRO_PASSWORD_LOOKUP "SELECT password FROM users WHERE username = ?;"
#define MACRO_SELECT_SALT "SELECT salt FROM users WHERE username = ?;"

#define MACRO_CREATE_TABLE_GENRES "CREATE TABLE genres (genre_id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL);"
#define MACRO_CREATE_TABLE_SUBGENRES "CREATE TABLE subgenres (subgenre_id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, genre_id INTEGER, FOREIGN KEY(genre_id) REFERENCES genres(genre_id));"
#define MACRO_INSERT_IN_USERS_TABLE "INSERT INTO users (username, password, salt) VALUES (?, ?, ?);"
#define MACRO_CREATE_TABLE_AUTHORS "CREATE TABLE IF NOT EXISTS authors (author_id INTEGER PRIMARY KEY AUTOINCREMENT,name TEXT NOT NULL );"
#define MACRO_CREATE_TABLE_USERS "CREATE TABLE IF NOT EXISTS users (user_id INTEGER PRIMARY KEY AUTOINCREMENT,username TEXT NOT NULL,password TEXT NOT NULL );"
#define MACRO_CREATE_TABLE_BOOKS "CREATE TABLE IF NOT EXISTS books (book_id INTEGER PRIMARY KEY AUTOINCREMENT,title TEXT NOT NULL,author_id INTEGER,subgenre_id INTEGER,rating REAL,ISBN TEXT NOT NULL,file_path TEXT,editura TEXT,description TEXT,FOREIGN KEY(author_id) REFERENCES authors(author_id),FOREIGN KEY(subgenre_id) REFERENCES subgenres(subgenre_id) );"
#define MACRO_CREATE_TABLE_USER_HISTORY "CREATE TABLE IF NOT EXISTS userHistory (book_id INTEGER PRIMARY KEY AUTOINCREMENT,user_id INTEGER,action TEXT NOT NULL,timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,FOREIGN KEY(user_id) REFERENCES users(user_id),FOREIGN KEY(book_id) REFERENCES books(book_id) );"
#define MACRO_HOW_MANY_TIMES_THE_BOOK_WAS_RATED "SELECT COUNT(*) FROM userHistory WHERE action = 'rate' AND book_id = ?;"
#define MACRO_GET_TOTAL_RATING "SELECT SUM(rating) FROM books WHERE book_id = ?;"    

//generally speaking -1 means internal error from sqlite

struct clientInfo{
    int clientFd;
    bool isLoggedIn;
};

//-1 : error or didn't find user;
int getUserId(sqlite3 *db, const char *username) {
    sqlite3_stmt *stmt;
    const char *sqlSelect = MACRO_GET_USERID;

    if (sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int userId = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
            return userId;
        } else {
            fprintf(stderr, "User not found\n");
            sqlite3_finalize(stmt);
            return -1;  // User not found
        }
    } else {
        fprintf(stderr, "Error preparing getUserId statement\n");
        return -1;
    }
}

//-1 : error or didn't find id;
int getBookId(sqlite3 *db, const char *bookTitle) {
    sqlite3_stmt *stmt;
    const char *sqlSelect = MACRO_GET_BOOKID;

    if (sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, bookTitle, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int bookId = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
            return bookId;
        } else {
            fprintf(stderr, "Book not found\n");
            sqlite3_finalize(stmt);
            return -1;  // Book not found
        }
    } else {
        fprintf(stderr, "Error preparing getBookId statement\n");
        return -1;
    }
}

//-1 eroare; 1 - it's fine
int insertUserHistoryForDownload(sqlite3 *db, int bookId, int userId, const char *action) {
    const char *sqlInsert = MACRO_INSERT_INTO_USER_HISTORY_FOR_DOWNLOAD;

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        sqlite3_bind_int(stmt, 2, bookId);
        sqlite3_bind_text(stmt, 3, action, -1, SQLITE_STATIC);

        time_t currentTime;
        time(&currentTime);
        char timestamp[20]; 
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&currentTime));
        sqlite3_bind_text(stmt, 4, timestamp, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE && sqlite3_errcode(db) != SQLITE_CONSTRAINT) {
            fprintf(stderr, "Error inserting into userHistory: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }

        sqlite3_finalize(stmt);
        return 1;  // Insertion successful
    } else {
        fprintf(stderr, "Error preparing userHistory insert statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
}

int sendFile(int clientFd, const char *filename) {

    int file = open(filename, O_RDONLY);
    if (file == -1) {
        perror("open");
        close(clientFd);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    //what i read() in binary mode, i send()
    while ((bytesRead = read(file, buffer, sizeof(buffer))) > 0) {
         if (send(clientFd, buffer, bytesRead, 0) == -1) {
            perror("send");
            close(file);
            close(clientFd);
            return -1;
        }
    }
    
    const char* terminationMessage = "FILE_TRANSFER_COMPLETE";
    send(clientFd, terminationMessage, strlen(terminationMessage), 0);

    close(file);
    return 1;
}

struct clientInfo clients[FD_SIZE];
int serverRunning = 1;

void removeNewLine(char *str){
    int newLinePos = strcspn(str,"\n");
    str[newLinePos] = '\0';
}

void clearBuffer(char buffer[BUFFER_SIZE]){
    memset(buffer,0,BUFFER_SIZE);
}

void clearSmallBuffer(char buffer[SMALL_BUFFER_SIZE]){
    memset(buffer,0,SMALL_BUFFER_SIZE);
}

sqlite3* openDatabase(const char* dbName){
    sqlite3 *db;
    int openResult = sqlite3_open(dbName,&db);

    if(openResult){
        perror("Can't open database");
        perror(sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    return db;
}

void closeDatabase(sqlite3 *db){
    sqlite3_close(db);
}

int createTableUserHistory(sqlite3 *db){

    char *zErrMsg = 0;
    const char *sql = MACRO_CREATE_TABLE_USER_HISTORY;

    if(sqlite3_exec(db,sql,0,0,&zErrMsg) == SQLITE_OK){
        printf("userHistory table successfully created \n");
        return 1;
    } else {
        perror(zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
}

int createTableBooks(sqlite3 *db){

    char *zErrMsg = 0;
    const char *sql = MACRO_CREATE_TABLE_BOOKS;

    if(sqlite3_exec(db,sql,0,0,&zErrMsg) == SQLITE_OK){
        printf("Books table successfully created \n");
        return 1;
    } else {
        perror(zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
}

int createTableUsers(sqlite3 *db){

    char *zErrMsg = 0;
    const char *sql = MACRO_CREATE_TABLE_USERS;

    if(sqlite3_exec(db,sql,0,0,&zErrMsg) == SQLITE_OK){
        printf("Users table successfully created \n");
        return 1;
    } else {
        perror(zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
}

int createTableAuthors(sqlite3 *db){

    char *zErrMsg = 0;
    const char *sql = MACRO_CREATE_TABLE_AUTHORS;

    if(sqlite3_exec(db,sql,0,0,&zErrMsg) == SQLITE_OK){
        printf("Authors table successfully created \n");
        return 1;
    } else {
        perror(zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
}

int createTableGenres(sqlite3 *db){

    char *zErrMsg = 0;
    const char *sql = MACRO_CREATE_TABLE_GENRES;

    if(sqlite3_exec(db,sql,0,0,&zErrMsg) == SQLITE_OK){
        printf("Genres table successfully created \n");
        return 1;
    } else {
        perror(zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
}

int createTableSubgenres(sqlite3 *db){

    char *zErrMsg = 0;
    const char *sql = MACRO_CREATE_TABLE_SUBGENRES;

    if(sqlite3_exec(db,sql,0,0,&zErrMsg) == SQLITE_OK){
        printf("Subgenres table successfully created \n");
        return 1;
    } else {
        perror(zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
}

char *generateSalt() {
    if (sodium_init() < 0) {
        perror("Failed to initialize libsodium");
        return NULL;
    }

    char *salt = malloc(SALT_SIZE);
    if (salt == NULL) {
        perror("Memory allocation error");
        return NULL;
    }
    randombytes_buf(salt, SALT_SIZE);

    return salt;
}

char *encryptPasswordWithSalt(const char *password, const char *salt) {
    if (sodium_init() < 0) {
        perror("Failed to initialize libsodium");
        return NULL;
    }

    char *hashedPassword = malloc(128);
    if (hashedPassword == NULL) {
        perror("Memory allocation error");
        return NULL;
    }

    if (crypto_pwhash_str(hashedPassword, password, strlen(password),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE,
                          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        perror("Error hashing the password\n");
        free(hashedPassword);
        return NULL;
    }

    return hashedPassword;
}

int insertInUsersTable(sqlite3 *db, const char *username, const char *password) {
    
    sqlite3_stmt *stmt;
    const char *salt = generateSalt();
    char *encryptedPassword = encryptPasswordWithSalt(password, salt);

    const char *sql = MACRO_INSERT_IN_USERS_TABLE;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, encryptedPassword, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, salt, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            perror(sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            free(encryptedPassword);

            return -1; // Insertion failed
        }

        free(encryptedPassword);
        sqlite3_finalize(stmt);
        return 1; // Insertion successful
    } else {
        perror(sqlite3_errmsg(db));
        free(encryptedPassword);

        return -1; //sql error
    }
}

char *getBestBooksByRating(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *query = MACRO_BEST_BOOKS_BY_RATING;

    char *resultBuffer = malloc(1024);

    if (resultBuffer == NULL) {
        perror("Memory allocation error");
        return NULL; 
    }

    resultBuffer[0] = '\0';
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (strlen(resultBuffer) + 50 > 1024) {
                perror("Result buffer is not large enough.\n");
                break;
            }

            char bookInfo[100];
            snprintf(bookInfo, sizeof(bookInfo), "Book ID: %d, Title: %s, Rating: %f\n",
                     sqlite3_column_int(stmt, 0),
                     sqlite3_column_text(stmt, 1),
                     sqlite3_column_double(stmt, 4));

            strcat(resultBuffer, bookInfo);
        }

        sqlite3_finalize(stmt);
    } else {
       perror(sqlite3_errmsg(db));
    }

    return resultBuffer;
}

char *getAllSubgenres(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *query = MACRO_ALL_SUBGENRES;
    char *result = malloc(1024);

    if (result == NULL) {
        perror("Memory allocation error");
        return NULL; 
    }

    result[0] = '\0';
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *subgenre = sqlite3_column_text(stmt, 0);

            if (subgenre != NULL) {
                if (strlen(result) + strlen(subgenre) + 30 > 1024) {
                    perror("Result buffer is not large enough.\n");
                    break;
                }

                strcat(result, "Subgenre :");
                strcat(result, subgenre);
                strcat(result, "\n");
            }
        }

        sqlite3_finalize(stmt);
    } else {    

        perror(sqlite3_errmsg(db));
    }

    return result;
}

char* getAllGenres(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *query = MACRO_ALL_GENRES;
    char *result = malloc(1024);

    if (result == NULL) {
        perror("Memory allocation error");
        return NULL; 
    }

    result[0] = '\0';
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *genre = sqlite3_column_text(stmt, 0);

            if (genre != NULL) {
                if (strlen(result) + strlen(genre) + 30 > 1024) {
                    perror("Result buffer is not large enough.\n");
                    break;
                }

                strcat(result, "Genre :");
                strcat(result, genre);
                strcat(result, "\n");
            }
        }

        sqlite3_finalize(stmt);
    } else {
        perror(sqlite3_errmsg(db));
    }

    return result;
}

char *getAllBooks(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *query = MACRO_ALL_BOOKS;
    char* resultBuffer = malloc(1024);

    if (resultBuffer == NULL) {
        perror("Memory allocation error");
        return NULL; 
    }

    resultBuffer[0] = '\0';
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *title = sqlite3_column_text(stmt, 1); //daca nu-i const ,primesti eroare
        
            if (title != NULL) {
                if (strlen(resultBuffer) + strlen(title) + 30 > 1024) {
                    perror("Result buffer is not large enough.\n");
                    break;
                }

                strcat(resultBuffer, "Title :");
                strcat(resultBuffer, title);
                strcat(resultBuffer, "\n");
            }
        }
        sqlite3_finalize(stmt);
    } else {
        perror(sqlite3_errmsg(db));
    }
    
    return resultBuffer;
}

char* getAllGenresWithSubgenres(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *query = MACRO_GENRES_WITH_SUBGENRES;
    char* resultBuffer = malloc(1024); 

    if (resultBuffer == NULL) {
        perror("Memory allocation error");
        return NULL;
    }

    resultBuffer[0] = '\0';

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *genre = sqlite3_column_text(stmt, 0);
            const char *subgenre = sqlite3_column_text(stmt, 1);

            if (subgenre != NULL) {
                if (strlen(resultBuffer) + strlen(genre) + strlen(subgenre) + 30 > 1024) {
                    perror("Result buffer is not large enough.\n");
                    break;
                }

                strcat(resultBuffer, "Genre: ");
                strcat(resultBuffer, genre);
                strcat(resultBuffer, ", Subgenre: ");
                strcat(resultBuffer, subgenre);
                strcat(resultBuffer, "\n");
            }
        }

        sqlite3_finalize(stmt);
    } else {
        perror(sqlite3_errmsg(db));
    }

    return resultBuffer;
}

char* getAllBooksByAuthor(sqlite3 *db, const char *authorName) {
    sqlite3_stmt *stmt;
    const char *query = MACRO_ALL_BOOKS_BY_AUTHOR;
    
    char *resultBuffer = malloc(1024);
    if (resultBuffer == NULL) {
        perror("Memory allocation error");
        return NULL; 
    }

    resultBuffer[0] = '\0';

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, authorName, -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int bookId = sqlite3_column_int(stmt, 0);
            const char *title = (const char *)sqlite3_column_text(stmt, 1);

            if (title != NULL) {
                if (strlen(resultBuffer) + strlen(title) + 30 > 1024) {
                    perror("Result buffer is not large enough.\n");
                    break;
                }

                strcat(resultBuffer, "Book ID: ");
                char bookIdStr[20];
                snprintf(bookIdStr, sizeof(bookIdStr), "%d", bookId);
                strcat(resultBuffer, bookIdStr);
                strcat(resultBuffer, ", Title: ");
                strcat(resultBuffer, title);
                strcat(resultBuffer, "\n");
            }
        }

        sqlite3_finalize(stmt);
    } else {
        perror(sqlite3_errmsg(db));
    }

    return resultBuffer;
}

char *getRecentlyAddedBooks(sqlite3 *db, int limit) {
    sqlite3_stmt *stmt;
    const char *query = MACRO_RECENTLY_ADDED_BOOKS;

    char *resultBuffer = malloc(1024);

    if (resultBuffer == NULL) {
        perror("Memory allocation error");
        return NULL; 
    }

    resultBuffer[0] = '\0';

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, limit);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (strlen(resultBuffer) + 100 > 1024) {
                perror("Result buffer is not large enough.\n");
                break;
            }

            char bookInfo[100];
            snprintf(bookInfo, sizeof(bookInfo), "Book ID: %d, Title: %s\n",
                     sqlite3_column_int(stmt, 0),
                     sqlite3_column_text(stmt, 1));

            strcat(resultBuffer, bookInfo);
        }

        sqlite3_finalize(stmt);
    } else {
        perror(sqlite3_errmsg(db));
    }

    return resultBuffer;
}

char* getAllDownloadedBooks(sqlite3 *db, int userId){
    const char* query = MACRO_DOWNLOADED_BOOKS_FOR_USER;
    sqlite3_stmt* stmt;

    char *buffer = malloc(1024);

    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }

    buffer[0] = '\0';
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* title = (const char*)sqlite3_column_text(stmt, 0);
            const char* author = (const char*)sqlite3_column_text(stmt, 1);

             snprintf(buffer + strlen(buffer), BUFFER_SIZE - strlen(buffer), "Title: %s, Author: %s\n", title, author);
        }

        sqlite3_finalize(stmt);
    } else {
        printf("Error preparing statement: %s\n", sqlite3_errmsg(db));
        free(buffer);
        return NULL;
    }

    return buffer;
}

char* rateBook(sqlite3* db, int bookId, int userId, const char* action, double rating) {

    if (rating < 0.01 || rating > 5) {
        return MACRO_RATING_OUT_OF_BOUNDS;
    }

    sqlite3_stmt* stmt;
    
    const char* query = MACRO_INSERT_IN_USER_HISTORY_FOR_RATING;

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        return MACRO_ERROR_SQL;
    }

    sqlite3_bind_int(stmt, 1, bookId);
    sqlite3_bind_int(stmt, 2, userId);
    sqlite3_bind_text(stmt, 3, action, -1, SQLITE_STATIC);

    time_t currentTime;
    time(&currentTime);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&currentTime));

    sqlite3_bind_text(stmt, 4, timestamp, -1, SQLITE_STATIC);

    if (strcmp(action, "rate") == 0) {
        // Check if the user has downloaded the book before allowing the rating
        const char* checkDownloadQuery = MACRO_DID_DOWNLOAD;
        sqlite3_stmt* checkDownloadStmt;
        int downloaded = 0;

        if (sqlite3_prepare_v2(db, checkDownloadQuery, -1, &checkDownloadStmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(checkDownloadStmt, 1, userId);
            sqlite3_bind_int(checkDownloadStmt, 2, bookId);

            if (sqlite3_step(checkDownloadStmt) == SQLITE_ROW) {
                downloaded = sqlite3_column_int(checkDownloadStmt, 0);
            }

            sqlite3_finalize(checkDownloadStmt);
        }

        const char* checkIfRatedQuery = MACRO_DID_RATE;
        sqlite3_stmt* checkRatingStmt;
        int rated = 0;

        if (downloaded > 0) {
            if (sqlite3_prepare_v2(db, checkIfRatedQuery, -1, &checkRatingStmt, NULL) == SQLITE_OK) {
                sqlite3_bind_int(checkRatingStmt, 1, userId);
                sqlite3_bind_int(checkRatingStmt, 2, bookId);

                if (sqlite3_step(checkRatingStmt) == SQLITE_ROW) {
                    rated = sqlite3_column_int(checkRatingStmt, 0);
                }

                sqlite3_finalize(checkRatingStmt);
            }

            if (rated == 0) {
                sqlite3_bind_double(stmt, 4, rating);

                // Update the book's rating in the books table
                const char* updateRatingQuery = "UPDATE books SET rating = rating + ? WHERE book_id = ?;";
                sqlite3_stmt* updateRatingStmt;

                if (sqlite3_prepare_v2(db, updateRatingQuery, -1, &updateRatingStmt, NULL) == SQLITE_OK) {
                    sqlite3_bind_double(updateRatingStmt, 1, rating);
                    sqlite3_bind_int(updateRatingStmt, 2, bookId);

                    if (sqlite3_step(updateRatingStmt) != SQLITE_DONE) {
                        fprintf(stderr, "Error updating book rating: %s\n", sqlite3_errmsg(db));
                        sqlite3_finalize(updateRatingStmt);
                        return MACRO_ERROR_SQL;
                    }

                    sqlite3_finalize(updateRatingStmt);
                } else {
                    fprintf(stderr, "Error preparing book rating update statement: %s\n", sqlite3_errmsg(db));
                    return MACRO_ERROR_SQL;
                }
            } else {
                sqlite3_bind_null(stmt, 4);
                return MACRO_ONLY_ONE_RATING_ALLOWED;
            }
        } else {
            return MACRO_MUST_DOWNLOAD_BOOK_BEFORE_RATING;
        }
    } else {
        sqlite3_bind_null(stmt, 4);
    }

    int result = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE) {
        return MACRO_ERROR_SQL;
    }

    return MACRO_SUCCESSFUL_RATING;
}

int getRatingsCount(sqlite3* db, int bookId){

    sqlite3_stmt* stmt;
    const char* query = MACRO_HOW_MANY_TIMES_THE_BOOK_WAS_RATED;

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, bookId);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int ratingCount = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
            return ratingCount;
        }

        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "Error preparing query: %s\n", sqlite3_errmsg(db));
    }

    return -1;
}

double getTotalRating(sqlite3* db, int bookId){
    sqlite3_stmt* stmt;
    const char* query = MACRO_GET_TOTAL_RATING;

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, bookId);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            double totalRating = sqlite3_column_double(stmt, 0);
            sqlite3_finalize(stmt);
            return totalRating;
        }

        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "Error preparing query: %s\n", sqlite3_errmsg(db));
    }

    return -1.0; 
}

double getAverageRating(sqlite3* db, int bookId) {

    int ratingCount = getRatingsCount(db,bookId);
    printf("ratingCount = '%d' \n", ratingCount);
    double totalRating = getTotalRating(db,bookId);
    printf("ratingCount = '%lf' \n", totalRating);

    if (ratingCount > 0) {
        return totalRating / ratingCount;
    } else {
        return 0;
    }
}

char* getAllAuthors(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *query = MACRO_ALL_AUTHORS;

    char *resultBuffer = malloc(1024);
    if (resultBuffer == NULL) {
        perror("Memory allocation error");
        return NULL; 
    }

    resultBuffer[0] = '\0';

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int authorId = sqlite3_column_int(stmt, 0);
            const char *authorName = (const char *)sqlite3_column_text(stmt, 1);

            if (authorName != NULL) {
                if (strlen(resultBuffer) + strlen(authorName) + 30 > BUFFER_SIZE) {
                    perror("Result buffer is not large enough.\n");
                    break;
                }

                strcat(resultBuffer, "Author ID: ");
                char authorIdStr[20];
                snprintf(authorIdStr, sizeof(authorIdStr), "%d", authorId);
                strcat(resultBuffer, authorIdStr);
                strcat(resultBuffer, ", Name: ");
                strcat(resultBuffer, authorName);
                strcat(resultBuffer, "\n");
            }
        }

        sqlite3_finalize(stmt);
    } else {
        perror(sqlite3_errmsg(db));
    }

    return resultBuffer;
}

//action needs to be 'download'
char *getMostPopularBooksBySearch(sqlite3 *db, const char *action) {
    sqlite3_stmt *stmt;
    const char *query = MACRO_MOST_POPULAR_BOOKS_BY_SEARCH;

    char *resultBuffer = malloc(1024);

    if (resultBuffer == NULL) {
        perror("Memory allocation error");
        return NULL; 
    }

    resultBuffer[0] = '\0';

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, action, -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (strlen(resultBuffer) + 100 > 1024) {
                perror("Result buffer is not large enough.\n");
                break;
            }

            char bookInfo[100];
            snprintf(bookInfo, sizeof(bookInfo), "Title: %s, Description: %s\n",
                     sqlite3_column_text(stmt, 0),
                     sqlite3_column_text(stmt, 1));

            strcat(resultBuffer, bookInfo);
        }

        sqlite3_finalize(stmt);
    } else {
        perror(sqlite3_errmsg(db));
    }

    return resultBuffer;
}
//action needs to be 'download'
char *getMostPopularAuthors(sqlite3 *db, const char *action) {
    sqlite3_stmt *stmt;
    const char *query = MACRO_MOST_POPULAR_AUTHORS;

    char *resultBuffer = malloc(1024);

    if (resultBuffer == NULL) {
        perror("Memory allocation error");
        return NULL; 
    }

    resultBuffer[0] = '\0';

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, action, -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (strlen(resultBuffer) + 100 > 1024) {
                perror("Result buffer is not large enough.\n");
                break;
            }

            char authorInfo[100];
            snprintf(authorInfo, sizeof(authorInfo), "AuthorName: %s, Genre: %s\n",
                     sqlite3_column_text(stmt, 0),
                     sqlite3_column_text(stmt, 1));

            strcat(resultBuffer, authorInfo);
        }

        sqlite3_finalize(stmt);
    } else {
        perror(sqlite3_errmsg(db));
    }

    return resultBuffer;
}

//action needs to be 'download'
char *getMostPopularSubgenres(sqlite3 *db, const char *action) {
    sqlite3_stmt *stmt;
    const char *query = MACRO_MOST_POPULAR_SUBGENRES;

    char *resultBuffer = malloc(1024);

    if (resultBuffer == NULL) {
        perror("Memory allocation error");
        return NULL; 
    }

    resultBuffer[0] = '\0';

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, action, -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (strlen(resultBuffer) + 100 > 1024) {
                perror("Result buffer is not large enough.\n");
                break;
            }

            char subgenreInfo[100];
            snprintf(subgenreInfo, sizeof(subgenreInfo), "Subgenre ID: %d, Name: %s, Search Count: %d\n",
                     sqlite3_column_int(stmt, 0),
                     sqlite3_column_text(stmt, 1),
                     sqlite3_column_int(stmt, 4));

            strcat(resultBuffer, subgenreInfo);
        }

        sqlite3_finalize(stmt);
    } else {
        perror(sqlite3_errmsg(db));
    }

    return resultBuffer;
}

//action needs to be 'download'
char *getMostPopularGenres(sqlite3 *db, const char *action) {
    sqlite3_stmt *stmt;
    const char *query = MACRO_MOST_POPULAR_GENRES;

    char *resultBuffer = malloc(1024);

    if (resultBuffer == NULL) {
        perror("Memory allocation error");
        return NULL; 
    }

    resultBuffer[0] = '\0';

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, action, -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (strlen(resultBuffer) + 100 > 1024) {
                perror("Result buffer is not large enough.\n");
                break;
            }

            char genreInfo[100];
            snprintf(genreInfo, sizeof(genreInfo), "Name: %s\n",
                     sqlite3_column_text(stmt, 1));

            strcat(resultBuffer, genreInfo);
        }

        sqlite3_finalize(stmt);
    } else {
        perror(sqlite3_errmsg(db));
    }

    return resultBuffer;
}

char* getRecommendedBooks(sqlite3* db, int userId) {
    char* resultBuffer = malloc(BUFFER_SIZE);
    if (resultBuffer == NULL) {
        perror("Memory allocation error");
        return NULL;
    }

    resultBuffer[0] = '\0';
    int genreId = 0;
    int subgenreId = 0;
    
    const char* firstQuery = MACRO_USER_MOST_DOWNLOADED_GENRE;
    sqlite3_stmt* firstStmt;
    if (sqlite3_prepare_v2(db, firstQuery, -1, &firstStmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(firstStmt, 1, userId);
        int result = sqlite3_step(firstStmt);
        if (result == SQLITE_ROW) {
            genreId = sqlite3_column_int(firstStmt, 0);
            subgenreId = sqlite3_column_int(firstStmt, 1);
        } else if (result != SQLITE_DONE) {
            fprintf(stderr, "Error executing query: %s\n", sqlite3_errmsg(db));
            free(resultBuffer);
            return NULL;
        }

        sqlite3_finalize(firstStmt);
    } else {
        fprintf(stderr, "Error preparing query: %s\n", sqlite3_errmsg(db));
        free(resultBuffer);
        return NULL;
    }

    const char* secondQuery = MACRO_GET_BOOKS_BY_GENRE_AND_SUBGENRE;
    sqlite3_stmt* secondStmt;
    if (sqlite3_prepare_v2(db, secondQuery, -1, &secondStmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(secondStmt, 1, genreId);
        sqlite3_bind_int(secondStmt, 2, subgenreId);

        int result;
        int count = 0;
        while ((result = sqlite3_step(secondStmt)) == SQLITE_ROW && count < 4) {
            const char* title = (const char*)sqlite3_column_text(secondStmt, 0);
            strncat(resultBuffer, title, BUFFER_SIZE - strlen(resultBuffer) - 1);
            strncat(resultBuffer, ",", BUFFER_SIZE - strlen(resultBuffer) - 1);
            count++;
        }

        if (result != SQLITE_DONE) {
            fprintf(stderr, "Error executing query: %s\n", sqlite3_errmsg(db));
            free(resultBuffer);
            return NULL;
        }
        sqlite3_finalize(secondStmt);
    } else {
        fprintf(stderr, "Error preparing query: %s\n", sqlite3_errmsg(db));
        free(resultBuffer);
        return NULL;
    }

    return resultBuffer;
}

const char *getPasswordForUser(sqlite3 *db, char username[SMALL_BUFFER_SIZE]) {
    sqlite3_stmt *stmt;
    const char *sql_select = MACRO_PASSWORD_LOOKUP;

    if (sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *storedPasswordColumn = (const char *)sqlite3_column_text(stmt, 0);
            char *password = strdup(storedPasswordColumn);

            if (password != NULL) {
                sqlite3_finalize(stmt);
                return password; // successful
            } else {
                perror("Memory allocation error");
                sqlite3_finalize(stmt);
                return MACRO_NO_INFORMATION_OR_ERROR;
            }
        }
    }

    return MACRO_NO_INFORMATION_OR_ERROR;
}

bool doesUserExist(sqlite3 *db, const char *username) {
    sqlite3_stmt *stmt;
    const char *sql_select = MACRO_SELECT_USERNAME;

    if (sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

        int step_result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (step_result == SQLITE_ROW) {
            return true;
        } else if (step_result == SQLITE_DONE) {
            return false;
        } else {
            perror(sqlite3_errmsg(db));
            return false;
        }
    } else {
        // Prepare statement error
        perror(sqlite3_errmsg(db));
        return false;
    }
}

const char *SELECT_SALT_FOR_USER(sqlite3 *db, const char *username) {
    sqlite3_stmt *stmt;
    const char *sql_select = MACRO_SELECT_SALT;

    if (sqlite3_prepare_v2(db, sql_select, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *storedSaltColumn = (const char *)sqlite3_column_text(stmt, 0);
            char *salt = strdup(storedSaltColumn);

            if (salt != NULL) {
                sqlite3_finalize(stmt);
                return salt; // successful
            } else {
                perror("Memory allocation error");
                sqlite3_finalize(stmt);
                return MACRO_NO_INFORMATION_OR_ERROR;
            }
        } else {
            sqlite3_finalize(stmt);
            return MACRO_NO_INFORMATION_OR_ERROR;
        }
    } else {
        perror(sqlite3_errmsg(db));
        return MACRO_NO_INFORMATION_OR_ERROR;
    }
}

char *authenticateUser(sqlite3 *db, int clientFd, char username[SMALL_BUFFER_SIZE], char password[SMALL_BUFFER_SIZE]) {
    char *resultBuffer = malloc(BUFFER_SIZE);
    if (resultBuffer == NULL) {
        perror("Memory allocation error");
        return NULL;
    }

    resultBuffer[0] = '\0';
    char storedSalt[crypto_pwhash_STRBYTES];
    snprintf(storedSalt, crypto_pwhash_STRBYTES, "%s", SELECT_SALT_FOR_USER(db, username));

    bool userExists = doesUserExist(db, username);
    const char *storedPassword = getPasswordForUser(db, username);

    if (userExists) {
        // User found in the database
        if (strcmp(storedSalt,MACRO_NO_INFORMATION_OR_ERROR) != 0) {
             if (crypto_pwhash_str_verify(storedPassword, password, strlen(password)) == 0) {
                clients[clientFd].isLoggedIn = true;
                snprintf(resultBuffer, BUFFER_SIZE, "%s", SUCCESSFUL_AUTHENTICATION);
            } else {
                snprintf(resultBuffer, BUFFER_SIZE, "%s", INCORRECT_PASSWORD);
            }
        } else {
            // Error retrieving salt
            snprintf(resultBuffer, BUFFER_SIZE, "%s", ERROR_SALT_RETRIEVAL);
        }
    } else {
        // New user
        snprintf(resultBuffer, BUFFER_SIZE, "%s", NEW_USER_MESSAGE);
        clients[clientFd].isLoggedIn = true;
        insertInUsersTable(db, username, password);
    }

    return resultBuffer;
}

char *getFilePathFromTitle(sqlite3 *db, const char *title) {
    sqlite3_stmt *stmt;
    const char *sqlSelect = MACRO_GET_FILE_PATH_FROM_TITLE;

    if (sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, title, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *filePath = (const char *)sqlite3_column_text(stmt, 0);
            char *result = malloc(strlen(filePath) + 1);
            strcpy(result, filePath);

            sqlite3_finalize(stmt);
            return result;
        } else {
            sqlite3_finalize(stmt);
            return NULL; // Title not found
        }
    } else {
        perror(sqlite3_errmsg(db));
        return NULL; // Unexpected error
    }
}

char* executeRecommendationQuery(sqlite3* db, const char* query, ...) {
    char* resultBuffer = malloc(BUFFER_SIZE);
    if (resultBuffer == NULL) {
        perror("Memory allocation error");
        return NULL;
    }

    resultBuffer[0] = '\0';

    va_list args;
    va_start(args, query);

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        int paramIndex = 1;
        while (1) {
            const char* paramType = va_arg(args, const char*);
            if (paramType == NULL) {
                break;
            }

            if (strcmp(paramType, "int") == 0) {
                int paramValue = va_arg(args, int);
                sqlite3_bind_int(stmt, paramIndex, paramValue);
            } else if (strcmp(paramType, "text") == 0) {
                const char* paramValue = va_arg(args, const char*);
                sqlite3_bind_text(stmt, paramIndex, paramValue, -1, SQLITE_STATIC);
            }

            paramIndex++;
        }

        // Execute the query and concatenate results
        int result;
        int count = 0;
        while ((result = sqlite3_step(stmt)) == SQLITE_ROW && count < 4) {
            const char* title = (const char*)sqlite3_column_text(stmt, 0);
            strncat(resultBuffer, title, BUFFER_SIZE - strlen(resultBuffer) - 1);
            strncat(resultBuffer, ",", BUFFER_SIZE - strlen(resultBuffer) - 1);
            count++;
        }

        if (result != SQLITE_DONE) {
            fprintf(stderr, "Error executing query: %s\n", sqlite3_errmsg(db));
            free(resultBuffer);
            return NULL;
        }

        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "Error preparing query: %s\n", sqlite3_errmsg(db));
        free(resultBuffer);
        return NULL;
    }

    va_end(args);
    return resultBuffer;
}

void handleClient(int clientFd, int index){

    char promptingBuffer[SMALL_BUFFER_SIZE];
    char requestBuffer[BUFFER_SIZE];
    char sendingBuffer[BUFFER_SIZE];
    char name[SMALL_BUFFER_SIZE];
    char password[SMALL_BUFFER_SIZE];
    char bookName[SMALL_BUFFER_SIZE];
    double rating = 0.0;

    //clients[clientFd].isLoggedIn = true;

    bool prepareDownload = false;

    while(1){

        snprintf(promptingBuffer,BUFFER_SIZE,"%s",TYPE_COMMAND);
        ssize_t bytesSent = send(clientFd,promptingBuffer,sizeof(promptingBuffer),0);

        if(bytesSent <= 0){
            perror("send prompting message \n");
            close(clientFd);
            shutdown(clientFd,SHUT_RDWR);
            break;
        }

        ssize_t bytesReceived = recv(clientFd,requestBuffer,sizeof(requestBuffer),0);

        if(bytesReceived <= 0){
            perror("recv");
            close(clientFd);
            shutdown(clientFd,SHUT_RDWR);
            break;
        }

        printf("Server got '%s' \n",requestBuffer);
        removeNewLine(requestBuffer);
        
        if (strncmp(requestBuffer, LOGIN_PREFIX, strlen(LOGIN_PREFIX)) == 0) {
            char *passwordFromUser;
             if (sscanf(requestBuffer, "LOGIN : %[^,], PASSWORD : %[^\n]",name, password) == 2) {
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if(db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }

                printf("Hello '%s' , cu parola '%s' \n", name, password);

                if(clients[clientFd].isLoggedIn == true){
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", ILLEGAL_USE_OF_LOGIN);
                    send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                } else {
                    char *result = authenticateUser(db,clientFd,name,password);

                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", result);
                    send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);

                    clearBuffer(sendingBuffer);
                    free(result);
                    closeDatabase(db);
                }
            } else {
                // Failed to extract username and password
                snprintf(sendingBuffer, BUFFER_SIZE,"%s",INCORRECT_USE_OF_LOGIN);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            }
        } else if(strcmp(requestBuffer,MACRO_GET_MY_RECOMMENDATIONS) == 0){
            if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else {
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if (db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }

                int userId = getUserId(db, name);
                char *result = getRecommendedBooks(db,userId);
                if ((result != NULL) && result[0] == '\0'){
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                } else {
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", result);
                }

                send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                clearBuffer(sendingBuffer);

                free(result);
                closeDatabase(db);
            }

        }else if(strcmp(requestBuffer,MACRO_GET_MY_DOWNLOADS) == 0){
            if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else {
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if (db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }

                int userId = getUserId(db, name);
                char *result = getAllDownloadedBooks(db,userId);
                if ((result != NULL) && result[0] == '\0'){
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                } else {
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", result);
                }

                send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                clearBuffer(sendingBuffer);

                free(result);
                closeDatabase(db);
            }
        }else if(sscanf(requestBuffer,"GET RATING : %[^,]",bookName) == 1){ //to see a book's rating
            if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else {
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if(db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }

                int bookId = getBookId(db,bookName);
                double result = getAverageRating(db,bookId);
                if (result == 0){
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                } else {
                    snprintf(sendingBuffer, BUFFER_SIZE, "Cartea %s are rating-ul %f",bookName,result);
                }

                send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                clearBuffer(sendingBuffer);

                closeDatabase(db);
                clearSmallBuffer(bookName);
            }
        }else if(sscanf(requestBuffer, "RATE BOOK : %[^,], %lf", bookName, &rating) == 2){ //command to rate a book
            if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else {
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if(db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }

                int userId = getUserId(db,name);
                int bookId = getBookId(db,bookName);

                if(bookId == -1){
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                    send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                    clearBuffer(sendingBuffer);
                } else {
                    char *result = rateBook(db,bookId,userId,"rate",rating);
                    if ((result != NULL) && result[0] == '\0'){
                        snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                    } else {
                        snprintf(sendingBuffer, BUFFER_SIZE, "%s", result);
                    }

                    send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                    clearBuffer(sendingBuffer);
                    closeDatabase(db);
                    rating = 0.0;
                    clearSmallBuffer(bookName);
                }
            }
        }else if(strncmp(requestBuffer, RECOGNIZE_DOWNLOAD,strlen(RECOGNIZE_DOWNLOAD)) == 0){

            if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else {
                char* bookInfo = requestBuffer + strlen(RECOGNIZE_DOWNLOAD);
                char* title = strtok(bookInfo, ",");

                if(title != NULL){
                    printf("title  : '%s' \n",title);

                    sqlite3 *db = openDatabase(DATABASE_PATH);

                    if(db == NULL){
                        perror("Database creation failed \n");
                        exit(EXIT_FAILURE);
                    }

                    char *filePath = getFilePathFromTitle(db,title);
                    int transferStatus = sendFile(clientFd,filePath);
                
                    if(transferStatus != 1){
                        printf("Something wrong with sendFile()");
                        exit(EXIT_FAILURE);
                    }

                    int bookId = getBookId(db, title);
                    /*if(bookId == -1){
                        snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                        send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                        clearBuffer(sendingBuffer);
                    }*/ 

                    int userId = getUserId(db, name);

                    const char* checkDownloadQuery = MACRO_DID_DOWNLOAD;
                    sqlite3_stmt* checkDownloadStmt;
                    int downloaded = 0;

                    if (sqlite3_prepare_v2(db, checkDownloadQuery, -1, &checkDownloadStmt, NULL) == SQLITE_OK) {
                        sqlite3_bind_int(checkDownloadStmt, 1, userId);
                        sqlite3_bind_int(checkDownloadStmt, 2, bookId);

                        if (sqlite3_step(checkDownloadStmt) == SQLITE_ROW) {
                            downloaded = sqlite3_column_int(checkDownloadStmt, 0);
                        }
                        sqlite3_finalize(checkDownloadStmt);
                    }

                    if(downloaded == 0){
                        int insertionUserHistory = insertUserHistoryForDownload(db, bookId, userId,"download");
                            if(insertionUserHistory == -1){
                            perror("Failed to insert in user history after user download");
                            exit(EXIT_FAILURE);
                        }
                    }
                
                    free(filePath);
                    closeDatabase(db);
                } else {
                    snprintf(sendingBuffer,BUFFER_SIZE,"%s",INCORRECT_USE_OF_DOWNLOAD);
                    send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                    clearBuffer(sendingBuffer);
                }
            }
        } else if(strcmp(requestBuffer,GET_ALL_AUTHORS) == 0){

            if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else {
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if (db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }

                char *result = getAllAuthors(db);
                if ((result != NULL) && result[0] == '\0'){
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                } else {
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", result);
                }

                send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                clearBuffer(sendingBuffer);

                free(result);
                closeDatabase(db);
            }
        } else if(strcmp(requestBuffer,GET_BEST_BOOK_BY_RATING) == 0){

            if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else {
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if (db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }

                char *result = getBestBooksByRating(db);
                if ((result != NULL) && result[0] == '\0'){
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                } else{
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", result);
                }

                send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                clearBuffer(sendingBuffer);

                free(result);
                closeDatabase(db);
            }
        } else if(strcmp(requestBuffer,GET_SUBGENRES) == 0 && clients[clientFd].isLoggedIn == true){

            if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else {
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if (db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }

                char *result = getAllSubgenres(db);
                if ((result != NULL) && result[0] == '\0'){
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                } else{
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", result);
                }

                send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                clearBuffer(sendingBuffer);

                free(result);
                closeDatabase(db);
            }
        } else if(strcmp(requestBuffer,GET_GENRES) == 0){

            if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else {
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if(db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }

                char* result = getAllGenres(db);
                if ( (result != NULL) && result[0] == '\0') {   
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                } else {
                    snprintf(sendingBuffer,BUFFER_SIZE,"%s",result);
                }

                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);

                free(result);
                closeDatabase(db);
            }

        } else if(strcmp(requestBuffer,GET_ALL_BOOKS) == 0){

            if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else {
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if (db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }
                
                char *result = getAllBooks(db);
                if ((result != NULL) && result[0] == '\0'){
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                } else {
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", result);
                }

                send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                clearBuffer(sendingBuffer);

                free(result);
                closeDatabase(db);
            }

        } else if(strcmp(requestBuffer,GET_ALL_GENRES_AND_SUBGENRES) == 0){
            
            if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else {
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if (db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }

                char *result = getAllGenresWithSubgenres(db);
                if ((result != NULL) && result[0] == '\0'){
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                } else {
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", result);
                }

                send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                clearBuffer(sendingBuffer);

                free(result);
                closeDatabase(db);
            }
        } else if (strncmp(requestBuffer, GET_BOOKS_BY_AUTHOR_PREFIX, strlen(GET_BOOKS_BY_AUTHOR_PREFIX)) == 0) {
            
            if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else {
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if (db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }
                const char *authorName = requestBuffer + strlen(GET_BOOKS_BY_AUTHOR_PREFIX);
                char *result = getAllBooksByAuthor(db, authorName);

                if ((result != NULL) && result[0] == '\0'){
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                } else{
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", result);
                }

                send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                clearBuffer(sendingBuffer);

                free(result);
                closeDatabase(db);
            }
        } else if (strncmp(requestBuffer, GET_RECENTLY_ADDED_BOOKS, strlen(GET_RECENTLY_ADDED_BOOKS)) == 0 ) {

            if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else{
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if (db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }
                const char *numberStr = requestBuffer + strlen(GET_RECENTLY_ADDED_BOOKS);
                int number = atoi(numberStr);
                printf("number from getRecently added books = '%d' \n", number);

                char *result = getRecentlyAddedBooks(db, number);

                if ((result != NULL) && result[0] == '\0'){
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                } else{
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", result);
                }

                send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                clearBuffer(sendingBuffer);

                free(result);
                closeDatabase(db);
            }
        } else if(strcmp(requestBuffer,LOGOUT) == 0){
            snprintf(sendingBuffer, BUFFER_SIZE,"%s",LOGOUT_RESPONSE);
            send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
            
            clients[clientFd].isLoggedIn = false;
            clearBuffer(sendingBuffer);

        } else if(strcmp(requestBuffer,RECOGNIZE_QUIT) == 0){
            snprintf(sendingBuffer, BUFFER_SIZE,"%s",QUIT_RESPONSE);
            send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
            
            clients[clientFd].isLoggedIn = false;

            close(clientFd);
            shutdown(clientFd,SHUT_RDWR);
            clearBuffer(sendingBuffer);
        } else if(strcmp(requestBuffer, GET_MOST_POPULAR_AUTHORS) == 0){
              if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else {
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if (db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }
                
                char *result = getMostPopularAuthors(db,"download");
                if ((result != NULL) && result[0] == '\0'){
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                } else {
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", result);
                }

                send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                clearBuffer(sendingBuffer);

                free(result);
                closeDatabase(db);
            }
        } else if(strcmp(requestBuffer, GET_MOST_POPULAR_GENRES) == 0){
            if(clients[clientFd].isLoggedIn != true){
                snprintf(sendingBuffer,BUFFER_SIZE,"%s",NO_PRIVILEGES);
                send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
                clearBuffer(sendingBuffer);
            } else {
                sqlite3 *db = openDatabase(DATABASE_PATH);

                if (db == NULL){
                    perror("Database creation failed \n");
                    exit(EXIT_FAILURE);
                }
                
                char *result = getMostPopularGenres(db,"download");
                if ((result != NULL) && result[0] == '\0'){
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", MACRO_NO_INFORMATION_IN_DATABASE);
                } else {
                    snprintf(sendingBuffer, BUFFER_SIZE, "%s", result);
                }

                send(clientFd, sendingBuffer, sizeof(sendingBuffer), 0);
                clearBuffer(sendingBuffer);

                free(result);
                closeDatabase(db);
            }
        }else { //unknown command
            snprintf(sendingBuffer, BUFFER_SIZE,"%s",UNKNOWN_COMMAND);
            send(clientFd,sendingBuffer,sizeof(sendingBuffer),0);
            clearBuffer(sendingBuffer);
        }

        clearBuffer(requestBuffer);
    }

    clients[index].clientFd = -1;
    close(clientFd);
}

int main(){

    for(int i = 0; i < FD_SIZE; i++){
        clients[i].clientFd = -1;
        clients[i].isLoggedIn = false;
    }

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket == -1){
        perror("creation of network socket \n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT_NUMBER);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int optval = 1;
    if(setsockopt(serverSocket,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval)) == -1){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if(bind(serverSocket,(struct sockaddr*) &serverAddress,sizeof(serverAddress)) == -1){
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    if(listen(serverSocket,FD_SIZE) == -1){
        perror("listen()");
        exit(EXIT_FAILURE);
    }

    while(serverRunning){

        printf("Waiting for connections ...\n");
        socklen_t clientAddressLength = sizeof(struct sockaddr);
        int clientFd = accept(serverSocket,(struct sockaddr*) &serverAddress,&clientAddressLength);

        if(clientFd == -1){
            perror("accept error \n");
            continue;
        }

        int emptyIndex = -1;
        //trying to find an empty index in the array that holds the descriptors values
        for(int i = 0; i < FD_SIZE; i++){
            if(clients[i].clientFd == -1){
                emptyIndex = i;
                break;
            }
        }

        if(emptyIndex == -1){
            printf("Too many clients, couldn't find place in the array \n");
            close(clientFd);
        } else {
            clients[emptyIndex].clientFd = clientFd;

            pid_t q = fork();

            if (q < 0) {
                perror("fork");
                continue;
            } else if (q == 0) {
                if (clientFd == -1) {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }

                handleClient(clientFd, emptyIndex);
                exit(EXIT_SUCCESS);
            } else {
                clients[emptyIndex].clientFd = -1;
                close(clientFd);
            }
        }
    }

    close(serverSocket);
    exit(EXIT_SUCCESS);
}    
