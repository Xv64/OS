#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int sys_fork(void){
	return fork();
}

int sys_procexit(void){
	exit();
	return 0; // not reached
}

int sys_wait(void){
	return wait();
}

int sys_kill(void){
	int pid;

	if (argint(0, &pid) < 0)
		return -1;
	return kill(pid);
}

int sys_getpid(void){
	return proc->pid;
}

uintp sys_sbrk(void){
	uintp addr;
	uintp n;

	if (arguintp(0, &n) < 0)
		return -1;
	addr = proc->sz;
	if (growproc(n) < 0)
		return -1;
	return addr;
}

int sys_sleep(void){
	int n;
	uint ticks0;

	if (argint(0, &n) < 0)
		return -1;
	acquire(&tickslock);
	ticks0 = ticks;
	while (ticks - ticks0 < n) {
		if (proc->killed) {
			release(&tickslock);
			return -1;
		}
		sleep(&ticks, &tickslock);
	}
	release(&tickslock);
	return 0;
}

int sys_ticks(void) {
  uint tickcount;

  acquire(&tickslock);
  tickcount = ticks;
  release(&tickslock);
  return tickcount;
}

unsigned int sys_getppid(void) {
	return proc->parent->pid;
}

int sys_bless(void){
	int pid;

	if (argint(0, &pid) < 0)
		return -1;
	return bless(pid);
}

int sys_damn(void){
	int pid;

	if (argint(0, &pid) < 0)
		return -1;
	return damn(pid);
}

int sys_isblessed(void){
	int pid;

	if (argint(0, &pid) < 0)
		return -1;
	return isblessed(pid);
}

int sys_amblessed(void){
	return proc->blessed;
}

void sys_cpuhalt(void) {
	if(proc->blessed) {
		amd64_hlt();
	}
}

int sys_bfork(void){
	return bfork();
}

int sys_pstate(void) {
	int pid;

	if (argint(0, &pid) < 0)
		return -1;
	return pstate(pid);
}

int sys_pname(void) {
	char* buf;
	int pid;
	int n;

	if (argint(0, &pid) < 0 || argstr(1, &buf) < 0 || argint(2, &n) < 0) {
		return -1;
	}
	return pname(pid, buf, n);
}
