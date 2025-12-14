#include <fcntl.h>   
#include <unistd.h>     
#include <errno.h>     
#include <sys/types.h> 
#include <sys/stat.h>   

#define BUFFER_SIZE 4096 

int str_len(const char *str) 
{
    int len = 0;
    while (str[len] != '\0')
    {
        len++;
    } 
    return len;
}

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

    if (argc != 3) 
    {
        print_error("Error: Not in the format: ./my_copy NameSource NameTarget\n");
        return 1; 
    }

    s_file = open(argv[1], O_RDONLY);
    if (s_file == -1) 
    {
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

    if (access(argv[2], F_OK) == 0) 
    {
        while (1) 
        {
            const char *prompt_msg = "Target file exists. Overwrite? (y/n): ";
            write(STDOUT_FILENO, prompt_msg, str_len(prompt_msg));
        
            int res = read(STDIN_FILENO, &yes_or_no, 1);
            if (res <= 0)
                return 1; 

            if (yes_or_no == 'y') 
            {
                break;
            } 
            else if (yes_or_no == 'n') 
            {
                const char *cancel_msg = "Copying was canceled at the user's request.\n";
                write(STDOUT_FILENO, cancel_msg, str_len(cancel_msg));
                close(s_file); 
                return 0;
            }
        }
    }
    
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

    while ((bytes_read = read(s_file, buffer, BUFFER_SIZE)) > 0) 
    {
        bytes_written = write(t_file, buffer, bytes_read);
        
        if (bytes_written != bytes_read) 
        {
            print_error("Error: Write error to destination file.\n");
            close(s_file);
            close(t_file);
            return 1;
        }
    }

    if (bytes_read == -1) {
        print_error("Error: Failed to read from source file.\n");
    }

    close(s_file);
    close(t_file);

    return 0;
}