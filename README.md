# onlineLibrary
Concurrent Server-client architecture  with functionalities of an online library

Users connect with the following command : LOGIN : yourUsername, PASSWORD : yourPassword.

Once logged in, they can choose to see the authors and their written books (Get books by author : authorName),
check the available genres and subgenres (Get genres, Get subgenres), or together (Get genres and subgenres),
download a book (DOWNLOAD : bookName, authorName), rate a book (RATE BOOK : bookName), see the rating (GET RATING : bookName)
and to get some book recommendations based on their download history (Get my recommendations). 
An exhaustive list of all their downloaded books can be seen by using the following command : Get my downloads.

At any time, the SHUTDOWN command can be sent to end the connection.

!NOTA BENE : the books in the insert_books.sql have a filePath relative to the Downloads directory, and for this reason,
the value corresponding to the file path should be of the follwing type : /home/yourComputerUsername/Downloads/hereTheNameOfTheBook.

<script src="https://cdnjs.cloudflare.com/ajax/libs/clipboard.js/2.0.8/clipboard.min.js"></script>
<div id="sqlite-commands">
    <code>
        sqlite3 LibraryManager &lt; insert_genres_and_subgenres.sql
    </code>
    <button class="btn" data-clipboard-target="#sqlite-commands">Copy</button>
</div>

<div id="compile-commands">
    <code>
        severulAlaBun.c -o serverulAlaBunExe -lsodium -lsqlite3 <br>
        myClient.c -o myClientExe
    </code>
    <button class="btn" data-clipboard-target="#compile-commands">Copy</button>
</div>

<div id="run-commands">
    <code>
        ./serverulAlaBunExe &amp; ./myClientExe
    </code>
    <button class="btn" data-clipboard-target="#run-commands">Copy</button>
</div>

<script>
    new ClipboardJS('.btn');
</script>
 
