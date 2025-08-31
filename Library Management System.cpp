#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// ======================= Book Class =======================
class Book {
private:
    string title;
    string author;
    string isbn;
    bool available;

public:
    Book(string t = "", string a = "", string i = "", bool av = true)
        : title(t), author(a), isbn(i), available(av) {}

    // Getters
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    string getISBN() const { return isbn; }
    bool isAvailable() const { return available; }

    // Setters
    void setAvailability(bool status) { available = status; }

    // Save to file format
    string toFileString() const {
        return title + "|" + author + "|" + isbn + "|" + (available ? "1" : "0");
    }

    // Load from file line
    static Book fromFileString(const string& line) {
        size_t p1 = line.find("|");
        size_t p2 = line.find("|", p1 + 1);
        size_t p3 = line.find("|", p2 + 1);

        string t = line.substr(0, p1);
        string a = line.substr(p1 + 1, p2 - p1 - 1);
        string i = line.substr(p2 + 1, p3 - p2 - 1);
        bool av = (line.substr(p3 + 1) == "1");
        return Book(t, a, i, av);
    }
};

// ======================= LibraryUser Class =======================
class LibraryUser {
private:
    string userID;
    string name;
    vector<string> borrowedBooks; 

public:
    LibraryUser(string id = "", string n = "") : userID(id), name(n) {}

    string getUserID() const { return userID; }
    string getName() const { return name; }

    void borrowBook(string isbn) { borrowedBooks.push_back(isbn); }
    void returnBook(string isbn) {
        borrowedBooks.erase(remove(borrowedBooks.begin(), borrowedBooks.end(), isbn), borrowedBooks.end());
    }

    vector<string> getBorrowedBooks() const { return borrowedBooks; }

    string toFileString() const {
        string line = userID + "|" + name;
        for (auto& b : borrowedBooks) line += "|" + b;
        return line;
    }

    static LibraryUser fromFileString(const string& line) {
        vector<string> tokens;
        size_t start = 0, end = 0;
        while ((end = line.find("|", start)) != string::npos) {
            tokens.push_back(line.substr(start, end - start));
            start = end + 1;
        }
        tokens.push_back(line.substr(start));

        LibraryUser user(tokens[0], tokens[1]);
        for (size_t i = 2; i < tokens.size(); i++) user.borrowBook(tokens[i]);
        return user;
    }
};

// ======================= Library Class =======================
class Library {
private:
    vector<Book> books;
    vector<LibraryUser> users;

    const string booksFile = "books.txt";
    const string usersFile = "users.txt";

public:
    Library() {
        loadBooks();
        loadUsers();
    }

    // -------- File Persistence --------
    void loadBooks() {
        ifstream in(booksFile);
        string line;
        while (getline(in, line)) {
            books.push_back(Book::fromFileString(line));
        }
        in.close();
    }

    void saveBooks() {
        ofstream out(booksFile);
        for (auto& b : books) out << b.toFileString() << endl;
        out.close();
    }

    void loadUsers() {
        ifstream in(usersFile);
        string line;
        while (getline(in, line)) {
            users.push_back(LibraryUser::fromFileString(line));
        }
        in.close();
    }

    void saveUsers() {
        ofstream out(usersFile);
        for (auto& u : users) out << u.toFileString() << endl;
        out.close();
    }

    // -------- Book Operations --------
    void addBook(string title, string author, string isbn) {
        books.push_back(Book(title, author, isbn));
        saveBooks();
    }

    void removeBook(string isbn) {
        books.erase(remove_if(books.begin(), books.end(),
                              [&](Book& b) { return b.getISBN() == isbn; }),
                    books.end());
        saveBooks();
    }

    void displayAllBooks() {
        cout << "\n=== Books in Library ===\n";
        for (auto& b : books) {
            cout << "Title: " << b.getTitle()
                 << " | Author: " << b.getAuthor()
                 << " | ISBN: " << b.getISBN()
                 << " | Status: " << (b.isAvailable() ? "Available" : "Borrowed")
                 << endl;
        }
    }

    // -------- User Operations --------
    void registerUser(string id, string name) {
        users.push_back(LibraryUser(id, name));
        saveUsers();
    }

    void removeUser(string id) {
        users.erase(remove_if(users.begin(), users.end(),
                              [&](LibraryUser& u) { return u.getUserID() == id; }),
                    users.end());
        saveUsers();
    }

    void displayAllUsers() {
        cout << "\n=== Registered Users ===\n";
        for (auto& u : users) {
            cout << "ID: " << u.getUserID() << " | Name: " << u.getName() << endl;
        }
    }

    void displayBorrowedBooks(string userID) {
        for (auto& u : users) {
            if (u.getUserID() == userID) {
                cout << "\nBorrowed Books of " << u.getName() << ":\n";
                for (auto& isbn : u.getBorrowedBooks()) {
                    for (auto& b : books) {
                        if (b.getISBN() == isbn)
                            cout << b.getTitle() << " by " << b.getAuthor() << endl;
                    }
                }
                return;
            }
        }
        cout << "User not found.\n";
    }

    // -------- Borrow/Return --------
    void borrowBook(string isbn, string userID) {
        for (auto& b : books) {
            if (b.getISBN() == isbn && b.isAvailable()) {
                for (auto& u : users) {
                    if (u.getUserID() == userID) {
                        u.borrowBook(isbn);
                        b.setAvailability(false);
                        saveBooks();
                        saveUsers();
                        cout << u.getName() << " borrowed \"" << b.getTitle() << "\"\n";
                        return;
                    }
                }
            }
        }
        cout << "Borrowing failed. Book unavailable or user not found.\n";
    }

    void returnBook(string isbn, string userID) {
        for (auto& b : books) {
            if (b.getISBN() == isbn && !b.isAvailable()) {
                for (auto& u : users) {
                    if (u.getUserID() == userID) {
                        u.returnBook(isbn);
                        b.setAvailability(true);
                        saveBooks();
                        saveUsers();
                        cout << u.getName() << " returned \"" << b.getTitle() << "\"\n";
                        return;
                    }
                }
            }
        }
        cout << "Return failed. Check user/book details.\n";
    }
};

// ======================= Main Program =======================
int main() {
    Library library;

    library.addBook("1984", "George Orwell", "978-0451524935");
    library.addBook("To Kill a Mockingbird", "Harper Lee", "978-0061120084");

    library.registerUser("001", "Alice");
    library.registerUser("002", "Bob");

    library.borrowBook("978-0451524935", "001");
    library.borrowBook("978-0061120084", "002");

    library.displayAllBooks();
    library.displayAllUsers();

    library.returnBook("978-0451524935", "001");
    library.displayBorrowedBooks("001");

    return 0;
}
