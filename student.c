#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NAME_LEN 50
#define LINE_LEN 256
#define DATA_FILE "student.dat"

typedef struct Student
{
    int roll;
    char name[NAME_LEN];
    float percentage;
    struct Student *next;
} Student;

static Student *head = NULL;
static int dirty = 0;

static void trim(char *s)
{
    char *p = s;
    size_t n;

    while (*p && isspace((unsigned char)*p))
        p++;

    if (p != s)
        memmove(s, p, strlen(p) + 1);

    n = strlen(s);

    while (n > 0 && isspace((unsigned char)s[n - 1]))
        s[--n] = '\0';
}

static int eof_seen = 0;

static int read_line(const char *prompt, char *buf, size_t size)
{
    size_t len;

    if (prompt)
    {
        printf("%s", prompt);
        fflush(stdout);
    }

    if (fgets(buf, (int)size, stdin) == NULL)
    {
        buf[0] = '\0';
        eof_seen = 1;
        return 0;
    }

    len = strlen(buf);

    if (len > 0 && buf[len - 1] == '\n')
    {
        buf[len - 1] = '\0';
    }
    else
    {
        int c;

        while ((c = getchar()) != '\n' && c != EOF)
        {
        }
    }

    return 1;
}

static int read_int(const char *prompt, int lo, int hi, int *out)
{
    char buf[LINE_LEN];
    char *end;
    long v;

    for (;;)
    {
        if (!read_line(prompt, buf, sizeof(buf)))
            return 0;

        trim(buf);

        if (buf[0] == '\0')
        {
            puts("Value cannot be empty.");
            continue;
        }

        v = strtol(buf, &end, 10);

        if (*end != '\0')
        {
            puts("Please enter a whole number.");
            continue;
        }

        if (v < lo || v > hi)
        {
            printf("Please enter a value between %d and %d.\n", lo, hi);
            continue;
        }

        *out = (int)v;
        return 1;
    }
}

static int read_float(const char *prompt, double lo, double hi, float *out)
{
    char buf[LINE_LEN];
    char *end;
    double v;

    for (;;)
    {
        if (!read_line(prompt, buf, sizeof(buf)))
            return 0;

        trim(buf);

        if (buf[0] == '\0')
        {
            puts("Value cannot be empty.");
            continue;
        }

        v = strtod(buf, &end);

        if (*end != '\0')
        {
            puts("Please enter a number.");
            continue;
        }

        if (v < lo || v > hi)
        {
            printf("Please enter a value between %.2f and %.2f.\n",
                   lo, hi);
            continue;
        }

        *out = (float)v;
        return 1;
    }
}

static int read_name(const char *prompt, char *dst, size_t size)
{
    char buf[LINE_LEN];
    char *bar;

    for (;;)
    {
        if (!read_line(prompt, buf, sizeof(buf)))
            return 0;

        trim(buf);

        if (buf[0] == '\0')
        {
            puts("Name cannot be empty.");
            continue;
        }

        while ((bar = strchr(buf, '|')) != NULL)
            *bar = ' ';

        if (strlen(buf) >= size)
            buf[size - 1] = '\0';

        strcpy(dst, buf);
        return 1;
    }
}

static char read_choice(const char *prompt)
{
    char buf[LINE_LEN];

    if (!read_line(prompt, buf, sizeof(buf)))
        return 'e';

    trim(buf);

    return (char)tolower((unsigned char)buf[0]);
}

static void pause_screen(void)
{
    char buf[LINE_LEN];

    read_line("\nPress Enter to continue...", buf, sizeof(buf));
}

static int ci_contains(const char *hay, const char *needle)
{
    size_t nl = strlen(needle);

    if (nl == 0)
        return 1;

    for (; *hay; hay++)
    {
        size_t i;

        for (i = 0; i < nl; i++)
        {
            if (tolower((unsigned char)hay[i]) !=
                tolower((unsigned char)needle[i]))
                break;

            if (hay[i] == '\0')
                break;
        }

        if (i == nl)
            return 1;
    }

    return 0;
}

static Student *find_by_roll(int roll)
{
    Student *p;

    for (p = head; p != NULL; p = p->next)
    {
        if (p->roll == roll)
            return p;
    }

    return NULL;
}

static int next_free_roll(void)
{
    int n;

    for (n = 1;; n++)
    {
        if (find_by_roll(n) == NULL)
            return n;
    }
}

static void list_append(Student *node)
{
    Student *p;

    node->next = NULL;

    if (head == NULL)
    {
        head = node;
    }
    else
    {
        p = head;

        while (p->next)
            p = p->next;

        p->next = node;
    }
}

static int list_count(void)
{
    int n = 0;
    Student *p;

    for (p = head; p; p = p->next)
        n++;

    return n;
}

static void list_free(void)
{
    Student *p = head;

    while (p)
    {
        Student *nxt = p->next;

        free(p);
        p = nxt;
    }

    head = NULL;
}

static void print_header(void)
{
    puts("--------------------------------------------------");
    puts("Roll No   Name                        Percentage");
    puts("--------------------------------------------------");
}

static void print_row(const Student *s)
{
    printf("%-9d %-27s %6.2f\n",
           s->roll,
           s->name,
           s->percentage);
}

static void stud_add(void)
{
    Student *node;

    puts("\n--- ADD NEW RECORD ---");

    node = (Student *)malloc(sizeof(Student));

    if (node == NULL)
    {
        puts("Out of memory.");
        return;
    }

    node->roll = next_free_roll();

    printf("Roll Number : %d\n", node->roll);

    if (!read_name("Student Name : ",
                   node->name,
                   NAME_LEN))
    {
        free(node);
        return;
    }

    if (!read_float("Percentage : ",
                    0.0,
                    100.0,
                    &node->percentage))
    {
        free(node);
        return;
    }

    list_append(node);

    dirty = 1;

    printf("\nRecord added successfully.\n");
}

static void delete_roll(int roll)
{
    Student *cur = head;
    Student *prev = NULL;

    while (cur && cur->roll != roll)
    {
        prev = cur;
        cur = cur->next;
    }

    if (cur == NULL)
    {
        printf("No record found with Roll No %d.\n", roll);
        return;
    }

    if (prev == NULL)
        head = cur->next;
    else
        prev->next = cur->next;

    printf("Deleted : Roll No %d - %s\n",
           cur->roll,
           cur->name);

    free(cur);

    dirty = 1;
}

static void del_by_roll(void)
{
    int roll;

    if (!read_int("Enter Roll Number to delete : ",
                  1,
                  1000000,
                  &roll))
        return;

    delete_roll(roll);
}

static void del_by_name(void)
{
    char key[NAME_LEN];
    Student *p;
    int matches = 0;
    int roll;

    if (!read_name("Enter Name : ",
                   key,
                   NAME_LEN))
        return;

    print_header();

    for (p = head; p; p = p->next)
    {
        if (ci_contains(p->name, key))
        {
            print_row(p);
            matches++;
        }
    }

    puts("--------------------------------------------------");

    if (matches == 0)
    {
        puts("No matching record.");
        return;
    }

    if (!read_int("Enter Roll Number to delete : ",
                  1,
                  1000000,
                  &roll))
        return;

    p = find_by_roll(roll);

    if (p == NULL || !ci_contains(p->name, key))
    {
        puts("Invalid Roll Number.");
        return;
    }

    delete_roll(roll);
}

static void stud_del(void)
{
    char ch;

    if (head == NULL)
    {
        puts("\nThe list is empty.");
        return;
    }

    puts("\n--- DELETE A RECORD ---");
    puts("R - Delete using Roll Number");
    puts("N - Delete using Name");
    puts("B - Back");

    ch = read_choice("Enter Your Choice : ");

    switch (ch)
    {
        case 'r':
            del_by_roll();
            break;

        case 'n':
            del_by_name();
            break;

        case 'b':
            break;

        default:
            puts("Invalid choice.");
    }
}

static void stud_show(void)
{
    Student *p;

    puts("\n--- STUDENT LIST ---");

    if (head == NULL)
    {
        puts("No records.");
        return;
    }

    print_header();

    for (p = head; p; p = p->next)
        print_row(p);

    puts("--------------------------------------------------");

    printf("Total records : %d\n", list_count());
}

static void modify_fields(Student *s)
{
    char ch;

    printf("\nRoll No : %d\n", s->roll);
    printf("Name    : %s\n", s->name);
    printf("Mark    : %.2f\n", s->percentage);

    puts("\nN - Modify Name");
    puts("P - Modify Percentage");
    puts("B - Back");

    ch = read_choice("Enter Your Choice : ");

    if (ch == 'n')
    {
        char newname[NAME_LEN];

        if (!read_name("New Name : ",
                       newname,
                       NAME_LEN))
            return;

        strcpy(s->name, newname);

        dirty = 1;

        puts("Name updated.");
    }
    else if (ch == 'p')
    {
        float pct;

        if (!read_float("New Percentage : ",
                        0.0,
                        100.0,
                        &pct))
            return;

        s->percentage = pct;

        dirty = 1;

        puts("Percentage updated.");
    }
    else if (ch != 'b')
    {
        puts("Invalid choice.");
    }
}

static Student *pick_from_matches(char mode)
{
    Student *p;
    int matches = 0;
    int roll;
    char key[NAME_LEN];
    float pct = 0.0f;

    if (mode == 'n')
    {
        if (!read_name("Enter Name : ",
                       key,
                       NAME_LEN))
            return NULL;
    }
    else
    {
        if (!read_float("Enter Percentage : ",
                        0.0,
                        100.0,
                        &pct))
            return NULL;
    }

    print_header();

    for (p = head; p; p = p->next)
    {
        int hit;

        if (mode == 'n')
        {
            hit = ci_contains(p->name, key);
        }
        else
        {
            hit = (p->percentage > pct - 0.005f &&
                   p->percentage < pct + 0.005f);
        }

        if (hit)
        {
            print_row(p);
            matches++;
        }
    }

    puts("--------------------------------------------------");

    if (matches == 0)
    {
        puts("No matching record.");
        return NULL;
    }

    if (!read_int("Enter Roll Number to modify : ",
                  1,
                  1000000,
                  &roll))
        return NULL;

    p = find_by_roll(roll);

    if (p == NULL)
    {
        puts("Invalid Roll Number.");
        return NULL;
    }

    return p;
}

static void stud_mod(void)
{
    char ch;
    Student *target = NULL;

    if (head == NULL)
    {
        puts("\nThe list is empty.");
        return;
    }

    puts("\n--- MODIFY A RECORD ---");
    puts("R - Search by Roll Number");
    puts("N - Search by Name");
    puts("P - Search by Percentage");
    puts("B - Back");

    ch = read_choice("Enter Your Choice : ");

    if (ch == 'r')
    {
        int roll;

        if (!read_int("Enter Roll Number : ",
                      1,
                      1000000,
                      &roll))
            return;

        target = find_by_roll(roll);

        if (target == NULL)
            puts("Record not found.");
    }
    else if (ch == 'n' || ch == 'p')
    {
        target = pick_from_matches(ch);
    }
    else if (ch == 'b')
    {
        return;
    }
    else
    {
        puts("Invalid choice.");
        return;
    }

    if (target)
        modify_fields(target);
}

static void stud_save(void)
{
    FILE *fp;
    Student *p;
    int n = 0;

    fp = fopen(DATA_FILE, "w");

    if (fp == NULL)
    {
        puts("Could not open file.");
        return;
    }

    for (p = head; p; p = p->next)
    {
        fprintf(fp,
                "%d|%.2f|%s\n",
                p->roll,
                p->percentage,
                p->name);

        n++;
    }

    fclose(fp);

    dirty = 0;

    printf("Saved %d record(s).\n", n);
}

static void stud_load(void)
{
    FILE *fp;
    char line[LINE_LEN];
    int n = 0;

    fp = fopen(DATA_FILE, "r");

    if (fp == NULL)
    {
        return;
    }

    while (fgets(line, sizeof(line), fp))
    {
        char *bar1;
        char *bar2;
        Student *node;

        line[strcspn(line, "\r\n")] = '\0';

        if (line[0] == '\0')
            continue;

        bar1 = strchr(line, '|');

        if (bar1 == NULL)
            continue;

        *bar1++ = '\0';

        bar2 = strchr(bar1, '|');

        if (bar2 == NULL)
            continue;

        *bar2++ = '\0';

        node = (Student *)malloc(sizeof(Student));

        if (node == NULL)
            break;

        node->roll = (int)strtol(line, NULL, 10);
        node->percentage = (float)strtod(bar1, NULL);

        strncpy(node->name,
                bar2,
                NAME_LEN - 1);

        node->name[NAME_LEN - 1] = '\0';

        if (node->roll <= 0 ||
            find_by_roll(node->roll))
        {
            free(node);
            continue;
        }

        list_append(node);

        n++;
    }

    fclose(fp);

    dirty = 0;

    printf("Loaded %d record(s).\n", n);
}

static int cmp_by_name(const Student *a,
                       const Student *b)
{
    const char *x = a->name;
    const char *y = b->name;

    while (*x && *y)
    {
        int cx = tolower((unsigned char)*x);
        int cy = tolower((unsigned char)*y);

        if (cx != cy)
            return cx - cy;

        x++;
        y++;
    }

    if (*x != *y)
        return (int)((unsigned char)*x) -
               (int)((unsigned char)*y);

    return a->roll - b->roll;
}

static int cmp_by_percentage(const Student *a,
                             const Student *b)
{
    if (a->percentage < b->percentage)
        return 1;

    if (a->percentage > b->percentage)
        return -1;

    return a->roll - b->roll;
}

static void sort_list(int (*cmp)(const Student *,
                                 const Student *))
{
    Student *sorted = NULL;
    Student *cur = head;

    while (cur)
    {
        Student *nxt = cur->next;

        if (sorted == NULL ||
            cmp(cur, sorted) < 0)
        {
            cur->next = sorted;
            sorted = cur;
        }
        else
        {
            Student *p = sorted;

            while (p->next &&
                   cmp(cur, p->next) >= 0)
                p = p->next;

            cur->next = p->next;
            p->next = cur;
        }

        cur = nxt;
    }

    head = sorted;
    dirty = 1;
}

static void stud_sort(void)
{
    char ch;

    if (head == NULL || head->next == NULL)
    {
        puts("\nNothing to sort.");
        return;
    }

    puts("\n--- SORT THE LIST ---");
    puts("N - Sort by Name");
    puts("P - Sort by Percentage");
    puts("B - Back");

    ch = read_choice("Enter Your Choice : ");

    if (ch == 'n')
    {
        sort_list(cmp_by_name);

        puts("List sorted by name.");

        stud_show();
    }
    else if (ch == 'p')
    {
        sort_list(cmp_by_percentage);

        puts("List sorted by percentage.");

        stud_show();
    }
    else if (ch != 'b')
    {
        puts("Invalid choice.");
    }
}

static int stud_exit(void)
{
    char ch;

    puts("\n--- EXIT ---");
    puts("S - Save and Exit");
    puts("E - Exit Without Saving");
    puts("B - Back");

    ch = read_choice("Enter Your Choice : ");

    if (ch == 's')
    {
        stud_save();
        return 1;
    }

    if (ch == 'e')
    {
        if (dirty)
        {
            char c;

            c = read_choice(
                "Unsaved changes. Discard? (y/n) : ");

            if (c != 'y')
                return 0;
        }

        return 1;
    }

    if (ch != 'b')
        puts("Invalid choice.");

    return 0;
}

static void show_menu(void)
{
    puts("\n**** STUDENT RECORD MENU ****");
    puts("A - Add New Record");
    puts("D - Delete a Record");
    puts("S - Show the List");
    puts("M - Modify a Record");
    puts("V - Save");
    puts("T - Sort the List");
    puts("E - Exit");
}

int main(void)
{
    int done = 0;

    puts("\n==================================");
    puts(" STUDENT RECORD MANAGEMENT SYSTEM");
    puts("==================================");

    stud_load();

    while (!done)
    {
        char ch;

        show_menu();

        ch = read_choice("Enter Your Choice : ");

        if (eof_seen)
            break;

        switch (ch)
        {
            case 'a':
                stud_add();
                pause_screen();
                break;

            case 'd':
                stud_del();
                pause_screen();
                break;

            case 's':
                stud_show();
                pause_screen();
                break;

            case 'm':
                stud_mod();
                pause_screen();
                break;

            case 'v':
                stud_save();
                pause_screen();
                break;

            case 't':
                stud_sort();
                pause_screen();
                break;

            case 'e':
                done = stud_exit();
                break;

            default:
                puts("Invalid choice.");
        }
    }

    list_free();

    puts("\nGoodbye.");

    return 0;
}