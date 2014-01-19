/*  This file is part of Lyos.

    Lyos is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Lyos is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lyos.  If not, see <http://www.gnu.org/licenses/>. */

#include "lyos/type.h"
#include "sys/types.h"
#include "stdio.h"
#include "unistd.h"
#include "assert.h"
#include "errno.h"
#include "fcntl.h"
#include "lyos/const.h"
#include "string.h"
#include "lyos/fs.h"
#include "lyos/proc.h"
#include "lyos/tty.h"
#include "lyos/console.h"
#include "lyos/global.h"
#include "lyos/keyboard.h"
#include "lyos/proto.h"
#include "termios.h"


#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

/* Default termios */
PRIVATE struct termios termios_defaults = {
  TINPUT_DEF, TOUTPUT_DEF, TCTRL_DEF, TLOCAL_DEF, TSPEED_DEF, TSPEED_DEF,
  {
	TEOF_DEF, TEOL_DEF, TERASE_DEF, TINTR_DEF, TKILL_DEF, TMIN_DEF,
	TQUIT_DEF, TTIME_DEF, TSUSP_DEF, TSTART_DEF, TSTOP_DEF,
	TREPRINT_DEF, TLNEXT_DEF, TDISCARD_DEF,
  },
};

PRIVATE void	init_tty	(TTY* tty);
PRIVATE void	tty_dev_read	(TTY* tty);
PRIVATE void	tty_dev_write	(TTY* tty);
PRIVATE void 	in_transfer	(TTY* tty);
PRIVATE void	tty_do_read	(TTY* tty, MESSAGE* msg);
PRIVATE void	tty_do_write	(TTY* tty, MESSAGE* msg);
PRIVATE void	tty_echo	(TTY* tty, char c);
PRIVATE void	put_key		(TTY* tty, u32 key);


/*****************************************************************************
 *                                task_tty
 *****************************************************************************/
/**
 * <Ring 1> Main loop of task TTY.
 *****************************************************************************/
PUBLIC void task_tty()
{
	TTY *	tty;
	MESSAGE msg;

	init_keyboard();

	for (tty = TTY_FIRST; tty < TTY_END; tty++)
		init_tty(tty);

	select_console(0);

	while (1) {
		for (tty = TTY_FIRST; tty < TTY_END; tty++) {
			do {
				handle_events(tty);
			} while (tty->ibuf_cnt);
		}
		
		send_recv(RECEIVE, ANY, &msg);

		int src = msg.source;
		assert(src != TASK_TTY);

		TTY* ptty = &tty_table[msg.DEVICE];

		switch (msg.type) {
		case DEV_OPEN:
			reset_msg(&msg);
			msg.type = SYSCALL_RET;
			send_recv(SEND, src, &msg);
			break;
		case DEV_READ:
			tty_do_read(ptty, &msg);
			break;
		case DEV_WRITE:
			tty_do_write(ptty, &msg);
			break;
		case HARD_INT:
			/**
			 * waked up by clock_handler -- a key was just pressed
			 * @see clock_handler() inform_int()
			 */
			key_pressed = 0;
			continue;
		default:
			dump_msg("TTY::unknown msg", &msg);
			break;
		}
	}
}


/*****************************************************************************
 *                                init_tty
 *****************************************************************************/
/**
 * Things to be initialized before a tty can be used:
 *   -# the input buffer
 *   -# the corresponding console
 * 
 * @param tty  TTY stands for teletype, a cool ancient magic thing.
 *****************************************************************************/
PRIVATE void init_tty(TTY* tty)
{
	tty->ibuf_cnt = 0;
	tty->ibuf_head = tty->ibuf_tail = tty->ibuf;

	tty->tty_termios = termios_defaults;

	init_screen(tty);
}


/*****************************************************************************
 *                                in_process
 *****************************************************************************/
/**
 * keyboard_read() will invoke this routine after having recognized a key press.
 * 
 * @param tty  The key press is for whom.
 * @param key  The integer key with metadata.
 *****************************************************************************/
PUBLIC void in_process(TTY* tty, u32 key)
{
	if (tty->tty_termios.c_iflag & ISTRIP) key &= 0x7F;

	if (tty->tty_termios.c_lflag & IEXTEN) {
		/* Previous character was a character escape? */
		if (tty->tty_escaped) {
			tty->tty_escaped = 0;
			put_key(tty, key);
			tty_echo(tty, key);
			return;
		}

		/* LNEXT (^V) to escape the next character? */
		if (key == tty->tty_termios.c_cc[VLNEXT]) {
			tty->tty_escaped = 1;
			tty_echo(tty, '^');
			tty_echo(tty, 'V');
			return;
		}
	}

	/* Map CR to LF, ignore CR, or map LF to CR. */
	if (key == '\r') {
		if (tty->tty_termios.c_iflag & IGNCR) return;
		if (tty->tty_termios.c_iflag & ICRNL) key = '\n';
	} else
	if (key == '\n') {
		if (tty->tty_termios.c_iflag & INLCR) key = '\r';
	}

	if (!(key & FLAG_EXT)) {
		put_key(tty, key);
		tty_echo(tty, key);
	}
	else {
		int raw_code = key & MASK_RAW;
		switch(raw_code) {
		case ENTER:
			put_key(tty, '\n');
			tty_echo(tty, '\n');
			break;
		case BACKSPACE:
			put_key(tty, '\b');
			if (tty->tty_trans_cnt) tty_echo(tty, '\b');
			break;
		case TAB:
			put_key(tty, '\t');
			tty_echo(tty, '\t');
			break;
		default:
			break;
		}
	}
}


/*****************************************************************************
 *                                put_key
 *****************************************************************************/
/**
 * Put a key into the in-buffer of TTY.
 *
 * @callergraph
 * 
 * @param tty  To which TTY the key is put.
 * @param key  The key. It's an integer whose higher 24 bits are metadata.
 *****************************************************************************/
PRIVATE void put_key(TTY* tty, u32 key)
{
	if (tty->ibuf_cnt < TTY_IN_BYTES) {
		*(tty->ibuf_head) = key;
		tty->ibuf_head++;
		if (tty->ibuf_head == tty->ibuf + TTY_IN_BYTES)
			tty->ibuf_head = tty->ibuf;
		tty->ibuf_cnt++;
	}
}


/*****************************************************************************
 *                                tty_dev_read
 *****************************************************************************/
/**
 * Get chars from the keyboard buffer if the TTY::console is the `current'
 * console.
 *
 * @see keyboard_read()
 * 
 * @param tty  Ptr to TTY.
 *****************************************************************************/
PRIVATE void tty_dev_read(TTY* tty)
{
	tty->tty_devread(tty);
}


/*****************************************************************************
 *                                tty_dev_write
 *****************************************************************************/
/**
 * Echo the char just pressed and transfer it to the waiting process.
 * 
 * @param tty   Ptr to a TTY struct.
 *****************************************************************************/
PRIVATE void tty_dev_write(TTY* tty)
{
	tty->tty_devwrite(tty);
}

/*****************************************************************************
 *                                in_transfer
 *****************************************************************************/
/**
 * Transfer chars to the waiting process.
 * 
 * @param tty   Ptr to a TTY struct.
 *****************************************************************************/
PRIVATE void in_transfer(TTY* tty)
{
	while (tty->ibuf_cnt) {
		char ch = *(tty->ibuf_tail);
		tty->ibuf_tail++;
		if (tty->ibuf_tail == tty->ibuf + TTY_IN_BYTES)
			tty->ibuf_tail = tty->ibuf;
		tty->ibuf_cnt--;

		if (tty->tty_inleft) {
			if (ch >= ' ' && ch <= '~') { /* printable */
				void * p = tty->tty_inbuf +
					   tty->tty_trans_cnt;
				phys_copy(p, (void *)va2la(TASK_TTY, &ch), 1);
				tty->tty_trans_cnt++;
				tty->tty_inleft--;
			}

			else if (ch == '\b' && tty->tty_trans_cnt) {
				tty->tty_trans_cnt--;
				tty->tty_inleft++;
			}

			if (ch == '\n' || tty->tty_inleft == 0) {
				MESSAGE msg;
				msg.type = tty->tty_inreply;
				msg.PROC_NR = tty->tty_inprocnr;
				msg.CNT = tty->tty_trans_cnt;
				send_recv(SEND, tty->tty_incaller, &msg);
				tty->tty_inleft = 0;
			}
		}
	}
}

/*****************************************************************************
 *                                handle_events
 *****************************************************************************/
/**
 * Handle all events pending on a tty.
 * 
 * @param tty   Ptr to a TTY struct.
 *****************************************************************************/
PUBLIC void handle_events(TTY * tty)
{
	do {
		tty_dev_read(tty);
		tty_dev_write(tty);
	} while (tty->tty_events);
	in_transfer(tty);
}

/*****************************************************************************
 *                                tty_do_read
 *****************************************************************************/
/**
 * Invoked when task TTY receives DEV_READ message.
 *
 * @note The routine will return immediately after setting some members of
 * TTY struct, telling FS to suspend the proc who wants to read. The real
 * transfer (tty buffer -> proc buffer) is not done here.
 * 
 * @param tty  From which TTY the caller proc wants to read.
 * @param msg  The MESSAGE just received.
 *
 * @see documentation/tty/
 *****************************************************************************/
PRIVATE void tty_do_read(TTY* tty, MESSAGE* msg)
{
	/* tell the tty: */
	tty->tty_inreply = SYSCALL_RET;
	tty->tty_incaller   = msg->source;  /* who called, usually FS */
	tty->tty_inprocnr   = msg->PROC_NR; /* who wants the chars */
	tty->tty_inbuf  = va2la(tty->tty_inprocnr,
				  msg->BUF);/* where the chars should be put */
	tty->tty_inleft = msg->CNT; /* how many chars are requested */
	tty->tty_trans_cnt= 0; /* how many chars have been transferred */

	msg->type = SUSPEND_PROC;
	msg->CNT = tty->tty_inleft;
	send_recv(SEND, tty->tty_incaller, msg);
	tty->tty_inreply = RESUME_PROC;
}


/*****************************************************************************
 *                                tty_do_write
 *****************************************************************************/
/**
 * Invoked when task TTY receives DEV_WRITE message.
 * 
 * @param tty  To which TTY the calller proc is bound.
 * @param msg  The MESSAGE.
 *****************************************************************************/
PRIVATE void tty_do_write(TTY* tty, MESSAGE* msg)
{
	/* tell the tty: */
	tty->tty_outreply    = SYSCALL_RET;
	tty->tty_outcaller   = msg->source;  /* who called, usually FS */
	tty->tty_outprocnr   = msg->PROC_NR; /* who wants to output the chars */
	tty->tty_outbuf  = va2pa(tty->tty_outprocnr,
				  msg->BUF);/* where are the chars */
	tty->tty_outleft = msg->CNT; /* how many chars are requested */
	tty->tty_outcnt = 0;

	handle_events(tty);
	if (tty->tty_outleft == 0) return;	/* already done, just return */

	if (msg->FLAGS & O_NONBLOCK) {	/* do not block */	
		tty->tty_outleft = tty->tty_outcnt = 0;
		msg->type = tty->tty_outreply;
		msg->STATUS = tty->tty_outcnt > 0 ? tty->tty_outcnt : EAGAIN;
		send_recv(SEND, tty->tty_outcaller, msg);
	} else {	/* block */
		msg->type = SUSPEND_PROC;
		msg->CNT = tty->tty_outleft;
		send_recv(SEND, tty->tty_outcaller, msg);
		tty->tty_outreply = RESUME_PROC;
	}
}

/*****************************************************************************
 *                                tty_echo
 *****************************************************************************/
/**
 * Echo the character is echoing is on.
 *
 *****************************************************************************/
PRIVATE void tty_echo(TTY* tty, char c)
{
	if (!(tty->tty_termios.c_lflag & ECHO)) return;
	tty->tty_echo(tty, c);
}

/*****************************************************************************
 *                                sys_printx
 *****************************************************************************/
/**
 * System calls accept four parameters. `printx' needs only two, so it wastes
 * the other two.
 *
 * @note `printx' accepts only one parameter -- `char* s', the other one --
 * `struct proc * proc' -- is pushed by kernel.asm::sys_call so that the
 * kernel can easily know who invoked the system call.
 *
 * @note s[0] (the first char of param s) is a magic char. if it equals
 * MAG_CH_PANIC, then this syscall was invoked by `panic()', which means
 * something goes really wrong and the system is to be halted; if it equals
 * MAG_CH_ASSERT, then this syscall was invoked by `assert()', which means
 * an assertion failure has occured. @see kernel/main lib/misc.c.
 * 
 * @param _unused1  Ignored.
 * @param _unused2  Ignored.
 * @param s         The string to be printed.
 * @param p_proc    Caller proc.
 * 
 * @return  Zero if success.
 *****************************************************************************/
PUBLIC int sys_printx(int _unused1, int _unused2, char* s, struct proc* p_proc)
{
	const char * p;
	char ch;

	char reenter_err[] = "? k_reenter is incorrect for unknown reason";
	reenter_err[0] = MAG_CH_PANIC;

	/**
	 * @note Code in both Ring 0 and Ring 1~3 may invoke printx().
	 * If this happens in Ring 0, no linear-physical address mapping
	 * is needed.
	 *
	 * @attention The value of `k_reenter' is tricky here. When
	 *   -# printx() is called in Ring 0
	 *      - k_reenter > 0. When code in Ring 0 calls printx(),
	 *        an `interrupt re-enter' will occur (printx() generates
	 *        a software interrupt). Thus `k_reenter' will be increased
	 *        by `kernel.asm::save' and be greater than 0.
	 *   -# printx() is called in Ring 1~3
	 *      - k_reenter == 0.
	 */
	if (k_reenter == 0)  /* printx() called in Ring<1~3> */
		p = va2la(proc2pid(p_proc), s);
	else if (k_reenter > 0) /* printx() called in Ring<0> */
		p = s;
	else	/* this should NOT happen */
		p = reenter_err;

	/**
	 * @note if assertion fails in any TASK, the system will be halted;
	 * if it fails in a USER PROC, it'll return like any normal syscall
	 * does.
	 */
	if ((*p == MAG_CH_PANIC) ||
	    (*p == MAG_CH_ASSERT && current < &proc_table[NR_TASKS])) {
		disable_int();
		char * v = (char*)V_MEM_BASE;
		const char * q = p + 1; /* +1: skip the magic char */

		while (v < (char*)(V_MEM_BASE + V_MEM_SIZE)) {
			*v++ = *q++;
			*v++ = RED_CHAR;
			if (!*q) {
				while (((int)v - V_MEM_BASE) % (SCR_WIDTH * 16)) {
					/* *v++ = ' '; */
					v++;
					*v++ = GRAY_CHAR;
				}
				q = p + 1;
			}
		}

		__asm__ __volatile__("hlt");
	}

	while ((ch = *p++) != 0) {
		if (ch == MAG_CH_PANIC || ch == MAG_CH_ASSERT)
			continue; /* skip the magic char */

		/* TTY * ptty; */
		/* for (ptty = TTY_FIRST; ptty < TTY_END; ptty++) */
		/* 	out_char(ptty->console, ch); /\* output chars to all TTYs *\/ */
		TTY_FIRST->tty_echo(TTY_FIRST, ch);
	}

	//__asm__ __volatile__("nop;jmp 1f;ud2;1: nop");
	//__asm__ __volatile__("nop;cli;1: jmp 1b;ud2;nop");

	return 0;
}

/*****************************************************************************
 *                                dump_tty_buf
 *****************************************************************************/
/**
 * For debug only.
 * 
 *****************************************************************************/
PUBLIC void dump_tty_buf()
{
	TTY * tty = &tty_table[1];

	static char sep[] = "--------------------------------\n";

	printl(sep);

	printl("head: %d\n", tty->ibuf_head - tty->ibuf);
	printl("tail: %d\n", tty->ibuf_tail - tty->ibuf);
	printl("cnt: %d\n", tty->ibuf_cnt);

	int pid = tty->tty_incaller;
	printl("caller: %s (%d)\n", proc_table[pid].name, pid);
	pid = tty->tty_inprocnr;
	printl("caller: %s (%d)\n", proc_table[pid].name, pid);

	printl("inbuf: %d\n", (int)tty->tty_inbuf);
	printl("left_cnt: %d\n", tty->tty_inleft);
	printl("trans_cnt: %d\n", tty->tty_trans_cnt);

	printl("--------------------------------\n");

	strcpy(sep, "\n");
}
