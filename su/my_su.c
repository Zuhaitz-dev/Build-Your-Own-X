// WARNING: FOR EDUCATIONAL PURPOSES ONLY. HUGE SECURITY RISK.

// Compilation: gcc my_su.c -o my_su -lcrypt
// Next steps:
//              sudo chown root:root my_su
//              sudo chmod u+s my_su

// Verify:
//              ls -l my_su

//  If you see something like:
//              -rwsr-xr-x 1 root root 16928 Aug 25 00:12 my_su
//  It's ready! We want the 's' (for setuid) to be present.

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // getuid, setuid, setgid, getpass, execlp
#include <crypt.h>
#include <sys/types.h>  // uid_t, gid_t
#include <pwd.h>        // getpwnam, struct passwd
#include <shadow.h>     // getspnam, struct spwd
#include <errno.h>

int main(int argc, char **argv)
{
    char            *target_user_name;
    struct passwd   *target_pw;
    struct spwd     *target_spw;
    char            *password_prompt;
    char            *entered_password;
    char            *encrypted_password;

    // First we check if we are running as root.
    if (0 != geteuid())
    {
        fprintf(stderr, "my_su: must be setuid root\n");
        return 1;
    }

    if (argc > 1)
    {
        target_user_name = argv[1];
    }
    else
    {
        target_user_name = "root";
    }

    // Now we get target user's info from /etc/passwd.
    errno = 0;
    target_pw = getpwnam(target_user_name);

    if (NULL == target_pw)
    {
        if (0 == errno)
        {
            fprintf(stderr, "my_su: user %s does not exist\n", target_user_name);
        }
        else
        {
            perror("getpwnam");
        }

        return 1;
    }

    // Next part is getting the encrypted password from /etc/shadow.
    target_spw = getspnam(target_user_name);

    if (NULL == target_spw)
    {
        perror("getspnam: cannot get shadow entry (do you have permission?)");
        return 1;
    }

    // Time for authentication!
    password_prompt = (char *) malloc(100); // Kind of an overkill, right? 
                                            // Maybe password_prompt[100]; would have been enough lol.
    snprintf(password_prompt, 100, "Password for %s: ", target_user_name);

    // WARNING: getpass is obsolete and unsafe, but simple.
    entered_password = getpass(password_prompt);

    // We use crypt() to hash the entered password ...
    encrypted_password = crypt(entered_password, target_spw->sp_pwdp);

    // We want to remove now the password from the memory.
    memset(entered_password, 0, strlen(entered_password));

    if (NULL == encrypted_password ||
        0 != strcmp(encrypted_password, target_spw->sp_pwdp))
    {
        fprintf(stderr, "my_su: Authentication failure\n");
        return 1;
    }

    printf("Authentication successful!\n");

    // We have to change the process identity now.
    // IMPORTANT: Change group ID *before* user ID.
    if (0 != setgid(target_pw->pw_gid))
    {
        perror("setgid failed");
        return 1;
    }
    if (0 != setuid(target_pw->pw_uid))
    {
        perror("setuid failed");
        return 1;
    }

    // Last but not least, we set a minimal environment and launch a new shell!
    setenv("HOME", target_pw->pw_dir, 1);
    setenv("USER", target_pw->pw_name, 1);
    setenv("LOGNAME", target_pw->pw_name, 1);
    setenv("SHELL", target_pw->pw_shell, 1);

    // The first argument is the program,
    // the second is how it sees itself (argv[0]),
    // the NULL terminates the argument list.
    execlp(target_pw->pw_shell, target_pw->pw_shell, (char *)NULL);

    perror("execlp failed");
    return 1;
}
