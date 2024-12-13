#include <stdio.h>
#include <stdlib.h>
#include <libudev.h>
#include <string.h>
#include <stddef.h>

#define LOG_FILE "usb_log.txt"
#define MAX_LOG_SIZE 256

#include <stddef.h>

char *strnstr(const char *haystack, const char *needle, size_t len)
{
	size_t i, j;

	if (!*needle)
		return (char *)haystack;
	for (i = 0; i < len && haystack[i] != '\0'; i++)
	{
		for (j = 0; needle[j] != '\0'; j++)
			if (i + j >= len || haystack[i + j] != needle[j])
				break;
		if (needle[j] == '\0')
			return ((char *)(haystack + i));
	}
	return (NULL);
}

void log_usb_info(const char *info)
{
	FILE *file = fopen(LOG_FILE, "a");
	if (!file)
	{
		perror("Failed to open log file");
		return;
	}
	if (strlen(info) > MAX_LOG_SIZE)
	{
		fprintf(stderr, "Log info is too large\n");
		return;
	}
	fprintf(file, "%s\n", info);
	fclose(file);
}

char *prepare_log(const char *action, const char *devnode, const char *vendor, const char *product)
{
	char *log_entry;

	log_entry = (char *)calloc(sizeof(char), 256);
	snprintf(log_entry, 255,
			 "Action: %s, Device Node: %s, Vendor: %s, Product: %s",
			 action ? action : "unknown",
			 devnode ? devnode : "unknown",
			 vendor ? vendor : "unknown",
			 product ? product : "unknown");
	log_entry[256] = '\0';
	return (log_entry);
}

int log_empty(const char *log_entry)
{
	int unknown_count = 0;
	const char *current_position = log_entry;
	size_t remaining_length = strlen(log_entry);

	while (remaining_length > 0 && (current_position = strnstr(current_position, "unknown", remaining_length)))
	{
		unknown_count++;
		size_t offset = current_position - log_entry + strlen("unknown");
		if (offset >= remaining_length)
			break ;
		current_position = log_entry + offset;
		remaining_length -= offset;
	}
	if (unknown_count >= 3)
		return (1);
	else
		return (0);
}

void process_usb_device(struct udev_device *dev)
{
	const char *action = udev_device_get_action(dev);
	const char *devnode = udev_device_get_devnode(dev);
	const char *vendor = udev_device_get_sysattr_value(dev, "idVendor");
	const char *product = udev_device_get_sysattr_value(dev, "idProduct");
	char *log_entry;

	if (action)
		if (strncmp(action, "remove", strlen("remove")) == 0)
			return ;
	log_entry = prepare_log(action, devnode, vendor, product);
	if (log_empty(log_entry) == 1)
		return ;
	log_usb_info(log_entry);
	printf("%s\n", log_entry);
	free(log_entry);
}

int main()
{
	struct udev *udev;
	struct udev_monitor *mon;
	struct udev_device *dev;

	udev = udev_new();
	if (!udev)
	{
		fprintf(stderr, "Failed to create udev context.\n");
		return (1);
	}
	mon = udev_monitor_new_from_netlink(udev, "udev");
	udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", NULL);
	udev_monitor_enable_receiving(mon);
	printf("Listening for USB devices...\n");
	while (1)
	{
		dev = udev_monitor_receive_device(mon);
		if (dev)
		{
			process_usb_device(dev);
			udev_device_unref(dev);
		}
	}
	udev_unref(udev);
	return (0);
}
