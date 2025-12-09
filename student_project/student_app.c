#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#ifdef _WIN32
  #include <conio.h>   // _getch()
#else
  #include <termios.h> // POSIX password masking
  #include <unistd.h>
#endif

#define STUD_FILE "students.txt"
#define CRE_FILE  "credentials.txt"

char currentUser[50];
char currentRole[20];

/* ================= PASSWORD MASKING (cross-platform) ================= */

void getPassword(char *pwd, size_t size) {
    size_t idx = 0;

#ifdef _WIN32
    int ch;
    while (1) {
        ch = _getch();              // does not echo
        if (ch == '\r' || ch == '\n' || ch == EOF) break;
        if ((ch == 8 || ch == 127) && idx > 0) { // backspace
            idx--;
            printf("\b \b");
            fflush(stdout);
        } else if (idx < size - 1 && ch >= 32 && ch <= 126) {
            pwd[idx++] = (char)ch;
            putchar('*');
            fflush(stdout);
        }
    }
    pwd[idx] = '\0';
    putchar('\n');

#else
    struct termios oldt, newt;

    if (tcgetattr(STDIN_FILENO, &oldt) != 0) {
        perror("tcgetattr");
        pwd[0] = '\0';
        return;
    }
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0) {
        perror("tcsetattr");
        pwd[0] = '\0';
        return;
    }

    while (1) {
        int ch = getchar();
        if (ch == '\n' || ch == '\r' || ch == EOF) break;

        if ((ch == 127 || ch == 8) && idx > 0) {
            idx--;
            printf("\b \b");
            fflush(stdout);
        } else if (idx < size - 1 && ch >= 32 && ch <= 126) {
            pwd[idx++] = (char)ch;
            printf("*");
            fflush(stdout);
        }
    }

    pwd[idx] = '\0';
    printf("\n");

    if (tcsetattr(STDIN_FILENO, TCSANOW, &oldt) != 0) {
        perror("tcsetattr restore");
    }
#endif
}

/* ================= LOGIN ================= */

int login() {
    char u[50], p[50], r[20];
    char inUser[50], inPass[50];

    printf("=====================================\n");
    printf("      STUDENT MANAGEMENT SYSTEM      \n");
    printf("=====================================\n\n");

    printf("USERNAME: ");
    if (scanf("%49s", inUser) != 1) return 0;

    // Clear leftover newline from buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }

    printf("PASSWORD: ");
    getPassword(inPass, sizeof(inPass));

    FILE *fp = fopen(CRE_FILE, "r");
    if (!fp) {
        printf("\n[ERROR] Credential file '%s' missing!\n", CRE_FILE);
        return 0;
    }

    while (fscanf(fp, "%49s %49s %19s", u, p, r) == 3) {
        if (strcmp(inUser, u) == 0 && strcmp(inPass, p) == 0) {
            strcpy(currentUser, u);
            strcpy(currentRole, r);
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

/* =============== STUDENT DATA HANDLING =============== */
/* File format (students.txt):
   roll|name with spaces allowed|mark
*/

typedef struct {
    int roll;
    char name[50];
    float mark;
} Student;

int readStudent(FILE *fp, Student *s) {
    char line[256];
    if (!fgets(line, sizeof(line), fp)) return 0;
    if (sscanf(line, "%d|%49[^|]|%f", &s->roll, s->name, &s->mark) == 3)
        return 1;
    return 0;
}

void writeStudent(FILE *fp, const Student *s) {
    fprintf(fp, "%d|%s|%.2f\n", s->roll, s->name, s->mark);
}

/* ================= CRUD OPERATIONS ================= */

void addStudent() {
    Student s;
    printf("\n--- Add Student ---\n");
    printf("Roll: ");
    if (scanf("%d", &s.roll) != 1) { while (getchar() != '\n'); printf("[WARN] Invalid roll\n"); return; }

    printf("Name: ");
    int c; while ((c = getchar()) != '\n' && c != EOF) { } // clear buffer
    if (!fgets(s.name, sizeof(s.name), stdin)) s.name[0] = '\0';
    s.name[strcspn(s.name, "\n")] = '\0';

    printf("Mark: ");
    if (scanf("%f", &s.mark) != 1) { while (getchar() != '\n'); printf("[WARN] Invalid mark\n"); return; }

    FILE *fp = fopen(STUD_FILE, "a");
    if (!fp) { printf("[ERROR] Cannot open '%s' for writing!\n", STUD_FILE); return; }
    writeStudent(fp, &s);
    fclose(fp);
    printf("[OK] Student added successfully!\n");
}

void displayStudents() {
    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) {
        printf("\n[INFO] No students found (file missing).\n");
        return;
    }

    char line[256];
    printf("\n--- Raw file lines (debug) ---\n");
    while (fgets(line, sizeof(line), fp)) {
        printf("LINE: %s", line);   // show exact raw line read (includes newline)
    }
    rewind(fp);

    // existing pretty print code follows...
    // (or run your normal readStudent loop afterwards)
    fclose(fp);
}


// helper: trim leading/trailing whitespace in-place
static void trim(char *s) {
    // trim leading
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);

    // trim trailing
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1])) s[--len] = '\0';
}

// helper: case-insensitive substring search
static int ci_str_contains(const char *hay, const char *needle) {
    if (!*needle) return 1;
    for (; *hay; ++hay) {
        const char *h = hay;
        const char *n = needle;
        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            h++; n++;
        }
        if (!*n) return 1; // found full needle
    }
    return 0;
}



void searchStudentByName() {
    char searchName[100];

    printf("\n--- Search Student (by Name) ---\n");
    printf("Enter name to search (partial ok): ");
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {} // clear buffer
    if (!fgets(searchName, sizeof(searchName), stdin)) {
        printf("[WARN] No input.\n");
        return;
    }
    trim(searchName);

    if (strlen(searchName) == 0) {
        printf("[WARN] Empty search term.\n");
        return;
    }

    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) {
        printf("[ERROR] Student file '%s' not found!\n", STUD_FILE);
        return;
    }

    Student s;
    int found = 0;
    while (readStudent(fp, &s)) {
        // use case-insensitive substring match on the stored name
        if (ci_str_contains(s.name, searchName)) {
            printf("Found: Roll=%d, Name=%s, Mark=%.2f\n", s.roll, s.name, s.mark);
            found = 1;
        }
    }

    if (!found) {
        printf("[INFO] No student with name matching '%s' found.\n", searchName);
    }

    fclose(fp);
}


void deleteStudentByRoll() {
    int delRoll;
    printf("\n--- Delete Student (by Roll) ---\n");
    printf("Enter roll to delete: ");
    if (scanf("%d", &delRoll) != 1) { while (getchar() != '\n'); printf("[WARN] Invalid roll\n"); return; }

    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) { printf("[ERROR] Student file '%s' not found!\n", STUD_FILE); return; }
    FILE *temp = fopen("temp.txt", "w");
    if (!temp) { printf("[ERROR] Cannot create temporary file!\n"); fclose(fp); return; }
    Student s; int found = 0;
    while (readStudent(fp, &s)) {
        if (s.roll != delRoll) writeStudent(temp, &s);
        else found = 1;
    }
    fclose(fp); fclose(temp);
    remove(STUD_FILE); rename("temp.txt", STUD_FILE);
    if (found) printf("[OK] Student with roll %d deleted!\n", delRoll);
    else printf("[INFO] Roll %d not found!\n", delRoll);
}

void updateStudentByRoll() {
    int updateRoll;
    printf("\n--- Update Student (by Roll) ---\n");
    printf("Enter roll to update: ");
    if (scanf("%d", &updateRoll) != 1) { while (getchar() != '\n'); printf("[WARN] Invalid roll\n"); return; }

    FILE *fp = fopen(STUD_FILE, "r");
    if (!fp) { printf("[ERROR] Student file '%s' not found!\n", STUD_FILE); return; }
    FILE *temp = fopen("temp.txt", "w");
    if (!temp) { printf("[ERROR] Cannot create temporary file!\n"); fclose(fp); return; }
    Student s; int found = 0;
    while (readStudent(fp, &s)) {
        if (s.roll == updateRoll) {
            found = 1;
            printf("Current: Roll=%d, Name=%s, Mark=%.2f\n", s.roll, s.name, s.mark);
            printf("New Name: ");
            int c; while ((c = getchar()) != '\n' && c != EOF) {}
            if (!fgets(s.name, sizeof(s.name), stdin)) s.name[0] = '\0';
            s.name[strcspn(s.name, "\n")] = '\0';
            printf("New Mark: ");
            if (scanf("%f", &s.mark) != 1) { while (getchar() != '\n'); printf("[WARN] Invalid mark\n"); }
            writeStudent(temp, &s);
        } else {
            writeStudent(temp, &s);
        }
    }
    fclose(fp); fclose(temp);
    remove(STUD_FILE); rename("temp.txt", STUD_FILE);
    if (found) printf("[OK] Student updated successfully!\n");
    else printf("[INFO] Roll %d not found!\n", updateRoll);
}

/* ================== MENUS ================== */

void adminMenu() {
    int c;
    while (1) {
        printf("\n========== ADMIN MENU ==========\n");
        printf("1. Add Student\n2. Display All Students\n3. Search Student (by Name)\n4. Update Student (by Roll)\n5. Delete Student (by Roll)\n6. Logout\n");
        printf("Enter choice: ");
        if (scanf("%d", &c) != 1) { while (getchar() != '\n'); printf("[WARN] Invalid choice\n"); continue; }
        switch (c) {
            case 1: addStudent(); break;
            case 2: displayStudents(); break;
            case 3: searchStudentByName(); break;
            case 4: updateStudentByRoll(); break;
            case 5: deleteStudentByRoll(); break;
            case 6: return;
            default: printf("[WARN] Invalid choice. Try again.\n");
        }
    }
}

void staffMenu() {
    int c;
    while (1) {
        printf("\n========== STAFF MENU ==========\n");
        printf("1. Add Student\n2. Display All Students\n3. Search Student (by Name)\n4. Update Student (by Roll)\n5. Logout\n");
        printf("Enter choice: ");
        if (scanf("%d", &c) != 1) { while (getchar() != '\n'); printf("[WARN] Invalid choice\n"); continue; }
        switch (c) {
            case 1: addStudent(); break;
            case 2: displayStudents(); break;
            case 3: searchStudentByName(); break;
            case 4: updateStudentByRoll(); break;
            case 5: return;
            default: printf("[WARN] Invalid choice. Try again.\n");
        }
    }
}

void guestMenu() {
    int c;
    while (1) {
        printf("\n========== GUEST MENU ==========\n");
        printf("1. Display All Students\n2. Search Student (by Name)\n3. Logout\n");
        printf("Enter choice: ");
        if (scanf("%d", &c) != 1) { while (getchar() != '\n'); printf("[WARN] Invalid choice\n"); continue; }
        switch (c) {
            case 1: displayStudents(); break;
            case 2: searchStudentByName(); break;
            case 3: return;
            default: printf("[WARN] Invalid choice. Try again.\n");
        }
    }
}

/* ================== MAIN ================== */

int main() {
    if (!login()) {
        printf("\n[ERROR] Invalid login!\n");
        return 0;
    }

    printf("\n[OK] Logged in as: %s (%s)\n", currentUser, currentRole);

    if (strcmp(currentRole, "admin") == 0) adminMenu();
    else if (strcmp(currentRole, "staff") == 0) staffMenu();
    else guestMenu();

    printf("\nGoodbye!\n");
    return 0;
}
