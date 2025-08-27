#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char filename[256];
    
    // Read filename from stdin if no arguments
    if (argc < 2) {
        if (fgets(filename, sizeof(filename), stdin) != NULL) {
            // Remove newline character
            filename[strcspn(filename, "\n")] = '\0';
        } else {
            printf("No filename provided\n");
            return 1;
        }
    } else {
        strncpy(filename, argv[1], sizeof(filename));
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("%s: failed to open %s\n", argv[0], filename);
        return 1;
    } else {
        printf("%s: successfully opened %s\n", argv[0], filename);
        fclose(file);
        return 0;
    }
}
