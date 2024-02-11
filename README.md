# onlineLibrary
Concurrent Server-client architecture  with functionalities of an online library

Users connect with the following command ``` LOGIN : yourUsername, PASSWORD : yourDesiredPassword```

Once logged in, they can choose to see the authors and their written books ```Get books by author : authorName```,
check the available genres and subgenres ```Get genres``` , ```Get subgenres```, or together ```Get genres and subgenres```,
download a book ```DOWNLOAD : bookName, authorName```, rate a book ```RATE BOOK : bookName```, see the rating ```GET RATING : bookName```
and to get some book recommendations based on their download history ```Get my recommendations```. 
An exhaustive list of all their downloaded books can be seen by using the following command ```Get my downloads```.

At any time, the ```SHUTDOWN``` command can be sent to end the connection.

!NOTA BENE : the books in the insert_books.sql have a filePath relative to the Downloads directory, and for this reason,
the value corresponding to the file path should be of the follwing type : /home/yourComputerUsername/Downloads/hereTheNameOfTheBook.
The database is created in the directory where you execute the ```Compilation Commands```

### SQLite commands for populating the database

```
sqlite3 LibraryManager < insert_genres_and_subgenres.sql
sqlite3 LibraryManager < insert_authors.sql
sqlite3 LibraryManager < insert_books.sql
```

### Compilation Commands

```
gcc severulAlaBun.c -o serverulAlaBunExe -lsodium -lsqlite3
gcc myClient.c -o myClientExe
```

### Run command
```
./serverulAlaBunExe & ./myClientExe
```
