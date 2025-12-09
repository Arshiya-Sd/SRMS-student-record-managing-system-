Student Record Manager with Role-Based Login

A simple and efficient Student Record Management System written in C, featuring role-based authentication (Admin, Staff, Guest) and complete CRUD operations using file handling.

This project demonstrates the use of:
Secure user authentication
File storage and retrieval
Structures
Modular programming
Input validation
Searching, updating, deleting records
Menu-driven UI

 Features-
 Role-Based Login

The system supports three types of users:

Role	Permissions
Admin	Add, Display, Search, Update, Delete
Staff	Add, Display, Search, Update
Guest	Display, Search
 Modules
1️. Login Module

Validates username & password from credentials.txt

Stores user role (admin/staff/guest)

Displays appropriate menu

2️. Student Management Module

Includes:

Add new student

Display all students (formatted output)

Search by name

Update using roll number

Delete using roll number

Persistent storage in students.txt

3️. File Handling

All data is stored in simple text files:

students.txt      → stores student records
credentials.txt   → stores username, password, role


Student record format:

roll|name|mark

Example:

1|Alice Johnson|85.50
2|Bob Kumar|72.25

 Example Output
=====================================
      STUDENT MANAGEMENT SYSTEM      
=====================================

USERNAME: admin
PASSWORD: *******

[OK] Logged in as: admin (admin)

========== ADMIN MENU ==========
1. Add Student
2. Display All Students
3. Search Student (By Name)
4. Update Student (By Roll)
5. Delete Student (By Roll)
6. Logout

 How to Compile & Run-
Compile
gcc student_app.c -o student_app

Run
./student_app


On Windows :

Press Run Code button
or

Use terminal:

.\student_app.exe

 File Structure-
student_project/
│
├── student_app.c
├── students.txt         
├── credentials.txt      
└── .gitignore           
 Future Enhancements-

These features can be added later:

Encrypted passwords

Sorting students by marks or name

Search by roll number

Admin password reset

GUI version using C++ or Python

 Author-

Arshiya Sd
Student Record Management System — Role Based Access
