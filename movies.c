#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct movie {
    char* title;
    int year;
    char* languages;
    double rating;
    struct movie* next;
};

/*
 * Create a movie node from one CSV line.
 * Format:
 * Title,Year,[Language1;Language2;...],Rating
 */
struct movie* createMovie(char* currLine) {
    struct movie* currMovie = malloc(sizeof(struct movie));

    char* saveptr;

    char* token = strtok_r(currLine, ",", &saveptr);
    currMovie->title = strdup(token);

    token = strtok_r(NULL, ",", &saveptr);
    currMovie->year = atoi(token);

    token = strtok_r(NULL, ",", &saveptr);
    currMovie->languages = strdup(token);

    token = strtok_r(NULL, ",", &saveptr);
    currMovie->rating = strtod(token, NULL);

    currMovie->next = NULL;

    return currMovie;
}

/*
 * Read the file, build a linked list in the same order as the file,
 * and print the processed message.
 */
struct movie* processFile(char* filePath) {
    FILE* movieFile = fopen(filePath, "r");
    if (movieFile == NULL) {
        printf("Could not open file %s\n", filePath);
        exit(1);
    }

    char* currLine = NULL;
    size_t len = 0;
    ssize_t nread;

    struct movie* head = NULL;
    struct movie* tail = NULL;

    int count = 0;

    /* Skip header line */
    nread = getline(&currLine, &len, movieFile);

    while ((nread = getline(&currLine, &len, movieFile)) != -1) {
        /* Make a copy because strtok_r modifies the string */
        char* lineCopy = strdup(currLine);

        struct movie* newMovie = createMovie(lineCopy);

        if (head == NULL) {
            head = newMovie;
            tail = newMovie;
        } else {
            tail->next = newMovie;
            tail = newMovie;
        }

        count++;
        free(lineCopy);
    }

    free(currLine);
    fclose(movieFile);

    printf("Processed file %s and parsed data for %d movies\n", filePath, count);

    return head;
}

/*
 * Option 1:
 * Show all movie titles released in the specified year.
 */
void showMoviesByYear(struct movie* list, int year) {
    int found = 0;
    struct movie* curr = list;

    while (curr != NULL) {
        if (curr->year == year) {
            printf("%s\n", curr->title);
            found = 1;
        }
        curr = curr->next;
    }

    if (!found) {
        printf("No data about movies released in the year %d\n", year);
    }
}

/*
 * Helper:
 * Return 1 if the movie contains the exact language, else 0.
 */
int hasLanguage(struct movie* m, char* language) {
    char* langsCopy = strdup(m->languages);
    char* saveptr;

    /* Remove leading '[' */
    if (langsCopy[0] == '[') {
        memmove(langsCopy, langsCopy + 1, strlen(langsCopy));
    }

    /* Remove trailing ']' if present */
    int len = strlen(langsCopy);
    if (len > 0 && langsCopy[len - 1] == ']') {
        langsCopy[len - 1] = '\0';
    }

    char* token = strtok_r(langsCopy, ";", &saveptr);
    while (token != NULL) {
        if (strcmp(token, language) == 0) {
            free(langsCopy);
            return 1;
        }
        token = strtok_r(NULL, ";", &saveptr);
    }

    free(langsCopy);
    return 0;
}

/*
 * Option 3:
 * Show year and title for all movies in a specified language.
 */
void showMoviesByLanguage(struct movie* list, char* language) {
    int found = 0;
    struct movie* curr = list;

    while (curr != NULL) {
        if (hasLanguage(curr, language)) {
            printf("%d %s\n", curr->year, curr->title);
            found = 1;
        }
        curr = curr->next;
    }

    if (!found) {
        printf("No data about movies released in %s\n", language);
    }
}

/*
 * Option 2:
 * Show highest rated movie for each year.
 *
 * This prints each year once, in the order that year first appears
 * in the file/list, which matches the sample behavior.
 */
void showHighestRatedByYear(struct movie* list) {
    struct movie* bestByYear[2022];
    int printed[2022];

    for (int i = 0; i < 2022; i++) {
        bestByYear[i] = NULL;
        printed[i] = 0;
    }

    struct movie* curr = list;

    /* First pass: find best movie for each year */
    while (curr != NULL) {
        int y = curr->year;
        if (bestByYear[y] == NULL || curr->rating > bestByYear[y]->rating) {
            bestByYear[y] = curr;
        }
        curr = curr->next;
    }

    /* Second pass: print each year once in order of first appearance */
    curr = list;
    while (curr != NULL) {
        int y = curr->year;
        if (!printed[y]) {
            printf("%d %.1f %s\n", y, bestByYear[y]->rating, bestByYear[y]->title);
            printed[y] = 1;
        }
        curr = curr->next;
    }
}

/*
 * Free the linked list.
 */
void freeMovies(struct movie* list) {
    struct movie* curr = list;

    while (curr != NULL) {
        struct movie* temp = curr;
        curr = curr->next;

        free(temp->title);
        free(temp->languages);
        free(temp);
    }
}

/*
 * Show menu until user chooses 4.
 */
void showMenu(struct movie* list) {
    int choice = 0;

    while (choice != 4) {
        printf("\n1. Show movies released in the specified year\n");
        printf("2. Show highest rated movie for each year\n");
        printf("3. Show the title and year of release of all movies in a specific language\n");
        printf("4. Exit from the program\n\n");
        printf("Enter a choice from 1 to 4: ");

        scanf("%d", &choice);

        if (choice == 1) {
            int year;
            printf("Enter the year for which you want to see movies: ");
            scanf("%d", &year);
            showMoviesByYear(list, year);
        } else if (choice == 2) {
            showHighestRatedByYear(list);
        } else if (choice == 3) {
            char language[21];
            printf("Enter the language for which you want to see movies: ");
            scanf("%20s", language);
            showMoviesByLanguage(list, language);
        } else if (choice == 4) {
            break;
        } else {
            printf("You entered an incorrect choice. Try again.\n");
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("You must provide the name of the file to process\n");
        return 1;
    }

    struct movie* list = processFile(argv[1]);
    showMenu(list);
    freeMovies(list);

    return 0;
}
