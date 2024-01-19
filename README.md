# onlineLibrary
Server-client architecture  with functionalities built around the functionalities of an online library

Users connect with the following command : LOGIN : yourUsername, PASSWORD : yourPassword.

Once logged in, they can choose to see the authors and their written books (Get books by author : authorName),
check the available genres and subgenres (Get genres, Get subgenres), or together (Get genres and subgenres),
download a book (DOWNLOAD : bookName, authorName), rate a book (RATE BOOK : bookName), see the rating (GET RATING : bookName)
and to get some book recommendations bassed on their download history (Get my recommendations). 
An exhaustive list of all their downloaded books can be seen by using the following command : Get my downloads.

At any time, the SHUTDOWN command can be sent to end the connection.

!NOTA BENE : the books in the insert_books.sql have a filePath relative to the Downloads directory, and for this reason,
the value corresponding to the file path should be of the follwing type : /home/yourComputerUsername/Downloads/hereTheNameOfTheBook.
 
