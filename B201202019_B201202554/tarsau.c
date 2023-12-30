#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_FILES 32
#define MAX_FILENAME_LENGTH 255
#define MAX_ARCHIVE_SIZE 200000000 // aprox. 200 MB 

void archiveFiles(char *outputFile, char files[][MAX_FILENAME_LENGTH], int fileCount);
void extractFiles(char *archiveFile, char *directory);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: tarsau -b [files] -o [output file] OR tarsau -a [archive file] [directory]\n");
        return 1;
    }

    if (strcmp(argv[1], "-b") == 0) {
        char files[MAX_FILES][MAX_FILENAME_LENGTH];
        char *outputFile = NULL;
        int i,fileCount = 0;
		
        for (i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                if (++i < argc) {
                    outputFile = argv[i];
                } else {
                    printf("Output file not specified.\n");
                    return 1;
                }
                break;
            } else {
                strncpy(files[fileCount++], argv[i], MAX_FILENAME_LENGTH);
                if (fileCount > MAX_FILES) {
                    printf("Too many files. The limit is %d files.\n", MAX_FILES);
                    return 1;
                }
            }
        }


        archiveFiles(outputFile, files, fileCount);
    } else if (strcmp(argv[1], "-a") == 0) {
        if (argc != 4) {
            printf("Usage: tarsau -a [archive file] [directory]\n");
            return 1;
        }

        char *archiveFile = argv[2];
        char *directory = argv[3];

        extractFiles(archiveFile, directory);
    } else {
        printf("Invalid command.\n");
        return 1;
    }

    return 0;
}
void archiveFiles(char *outputFile, char files[][MAX_FILENAME_LENGTH], int fileCount) {
    int i;
	FILE *archive = fopen(outputFile, "w");
    if (!archive) {
        printf("Failed to create archive file, exiting.\n");
        return;
    }

    for (i = 0; i < fileCount; i++) {
        FILE *file = fopen(files[i], "r");
        if (!file) {
            printf("Failed to open %s\n",files[i]);
            continue;
        }

        // Write metadata
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        fprintf(archive, "%s,%ld,", files[i], fileSize);

        // Write file contents
        char buffer;
        while ((buffer = fgetc(file)) != EOF) {
            fputc(buffer, archive);
        }

        fclose(file);
    }

    fclose(archive);
    printf("The files have been merged.\n");
}

void extractFiles(char *archiveFile, char *directory) {
    FILE *archive = fopen(archiveFile, "r");
    if (!archive) {
        perror("Failed to open archive file");
        return;
    }

    char filename[MAX_FILENAME_LENGTH];
    while (!feof(archive)) {
        long fileSize;
        int res = fscanf(archive, "%[^,],%ld,", filename, &fileSize);
        if (res != 2) break;

        char fullPath[MAX_FILENAME_LENGTH];
        // Check if the combined path length exceeds the limit
        if (snprintf(fullPath, sizeof(fullPath), "%s/%s", directory, filename) >= MAX_FILENAME_LENGTH) {
            printf("Path too long for file: %s\n", filename);
            continue;
        }

        FILE *extractedFile = fopen(fullPath, "w");
        if (!extractedFile) {
            perror("Failed to create extracted file");
            continue;
        }

        // Extract file contents
        for (long i = 0; i < fileSize; i++) {
            char buffer = fgetc(archive);
            if (feof(archive)) 
                break; // Check for end of file
            fputc(buffer, extractedFile);
        }

        fclose(extractedFile);
    }

    fclose(archive);
    printf("Files opened in the %s directory\n", directory);
}

