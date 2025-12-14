#include <fcntl.h>   
#include <unistd.h>     
#include <errno.h>     
#include <sys/types.h> 
#include <sys/stat.h>   

/* * Buffer Size Selection: 4096 bytes.
 * Justification: 4KB is the standard memory page size in Linux.
 * Reading and writing in chunks that align with the page size reduces 
 * the number of system calls and improves I/O efficiency.
 */
#define BUFFER_SIZE 4096 

// Helper function to calculate string length (replaces strlen from string.h)
int str_len(const char *str) 
{
    int len = 0;
    while (str[len] != '\0')
    {
        len++;
    } 
    return len;
}

// Helper function to write error messages to the Standard Error output (FD 2)
void print_error(const char *msg) 
{
    write(STDERR_FILENO, msg, str_len(msg));
}

int main(int argc, char* argv[]) 
{
    char buffer[BUFFER_SIZE];
    int s_file, t_file;
    int bytes_read, bytes_written;
    char yes_or_no;

    // Check arguments: We expect program name + source + destination (3 args)
    if (argc != 3) 
    {
        print_error("Error: Not in the format: ./my_copy NameSource NameTarget\n");
        return 1; 
    }

    /* * Open the source file for reading only (O_RDONLY).
     * open() returns a file descriptor (positive integer) on success,
     * or -1 if an error occurs.
     */
    s_file = open(argv[1], O_RDONLY);
    if (s_file == -1) 
    {
        // Check errno to determine the specific reason for failure
        if (errno == ENOENT) 
        {
            print_error("Error: The source file does not exist.\n");
        } 
        else if (errno == EACCES) 
        {
            print_error("Error: Permission denied for source file.\n");
        } 
        else 
        {
            print_error("Error: Failed to open source file.\n");
        }
        return 1;
    }

    /* * Check if the destination file already exists using access().
     * F_OK flag checks for existence. access() returns 0 if file exists.
     * This is done to prompt the user before overwriting.
     */
    if (access(argv[2], F_OK) == 0) 
    {
        while (1) 
        {
            const char *prompt_msg = "Target file exists. Overwrite? (y/n): ";
            write(STDOUT_FILENO, prompt_msg, str_len(prompt_msg));
        
            // Read 1 byte from Standard Input (keyboard) to get user response
            int res = read(STDIN_FILENO, &yes_or_no, 1);
            if (res <= 0)
                return 1; // Error or closed input

            if (yes_or_no == 'y') 
            {
                break; // Exit loop and proceed to overwrite
            } 
            else if (yes_or_no == 'n') 
            {
                const char *cancel_msg = "Copying was canceled at the user's request.\n";
                write(STDOUT_FILENO, cancel_msg, str_len(cancel_msg));
                close(s_file); // Don't forget to close the source file
                return 0;
            }
        }
    }
    
    /* * Open/Create destination file with flags:
     * O_WRONLY: Write-only mode.
     * O_CREAT: Create the file if it doesn't exist.
     * O_TRUNC: Truncate file length to 0 (overwrite content) if it exists.
     * 0644: Permissions (RW for owner, R for group/others).
     */
    t_file = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (t_file == -1) 
    {
        if (errno == EACCES) 
        {
            print_error("Error: Permission denied for destination directory/file.\n");
        }
        else if (errno == EISDIR) 
        {
            print_error("Error: Destination is a directory, cannot overwrite.\n");
        }
        else 
        {
            print_error("Error: Failed to open/create destination file.\n");
        }
        close(s_file);
        return 1;
    }

    /* * Main Copy Loop:
     * read() fills the buffer and returns the number of bytes read.
     * Return values:
     * > 0 : Number of bytes read (continue loop).
     * 0 : End of File (EOF) reached (stop loop).
     * -1 : Error occurred.
     */
    while ((bytes_read = read(s_file, buffer, BUFFER_SIZE)) > 0) 
    {
        // Write exactly the amount of bytes we read to the destination
        bytes_written = write(t_file, buffer, bytes_read);
        
        // Validate that all bytes were written successfully
        if (bytes_written != bytes_read) 
        {
            print_error("Error: Write error to destination file.\n");
            close(s_file);
            close(t_file);
            return 1;
        }
    }

    // If the loop exited because read() returned -1, handle the error
    if (bytes_read == -1) {
        print_error("Error: Failed to read from source file.\n");
    }

    // Close both file descriptors to release system resources
    close(s_file);
    close(t_file);

    return 0;
}