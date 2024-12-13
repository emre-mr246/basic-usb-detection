#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 

void play_sound()
{
    pid_t pid;

    pid = fork();
    if (pid != 0)
    {
        system("amixer set Master 100%");
        system("speaker-test -t sine -f 1000 -l 0 > /dev/null 2>&1 &");
        exit(0);
    }
}

void play_alert_sound()
{
    play_sound();
    system("xdg-screensaver lock");
    sleep(10);
}

int main()
{
    char buffer[1024];
    FILE *fp;

    fp = popen("udevadm monitor --subsystem-match=usb", "r");
    if (!fp)
    {
        perror("popen failed");
        return (1);
    }
    while (fgets(buffer, sizeof(buffer), fp))
    {
        if (strstr(buffer, "add"))
        {
            play_alert_sound();
            break ;
        }
    }
    pclose(fp);
    return (0);
}
