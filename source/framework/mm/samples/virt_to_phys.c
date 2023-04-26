#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
size_t virtual_to_physical(pid_t pid, size_t addr)
{
	char str[20];
	sprintf(str, "/proc/%u/pagemap", pid);
	int fd = open(str, O_RDONLY);
	if (fd < 0) {
		printf("open %s failed!\n", str);
		return 0;
	}
	size_t pagesize = getpagesize();
	size_t offset = (addr / pagesize) * sizeof(uint64_t);
	if (lseek(fd, offset, SEEK_SET) < 0) {
		printf("lseek() failed!\n");
		close(fd);
		return 0;
	}
	uint64_t info = 0;
	if (read(fd, &info, sizeof(uint64_t)) != sizeof(uint64_t)) {
		printf("read() failed!\n");
		close(fd);
		return 0;
	}
	if ((info & (((uint64_t)1) << 63)) == 0) {
		printf("page is not present!\n");
		close(fd);
		return 0;
	}
	size_t frame = info & ((((uint64_t)1) << 55) - 1);
	size_t phy = frame * pagesize + addr % pagesize;
	close(fd);
	printf("The phy frame is 0x%zx\n", frame);
	printf("The phy addr is 0x%zx\n", phy);
	return phy;
}
int main(void)
{
	while (1) {
		uint32_t pid;
		uint64_t virtual_addr;
		printf("Please input the pid in dec:");
		scanf("%u", &pid);
		printf("Please input the virtual address in hex:");
		scanf("%zx", &virtual_addr);
		printf("pid = %u and virtual addr = 0x%zx\n", pid,
		       virtual_addr);
		virtual_to_physical(pid, virtual_addr);
	}
	return 0;
}