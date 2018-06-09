#include <regx52.h>

char enFlicker = 0;
char flicker = 0;

unsigned long start_counting_time = 0;
unsigned long start_too_dry_time = 0;
unsigned int max_time = 1;
char is_too_dry = 0;

enum State {
	STATE_SHOWING,
	STATE_SETTING
} state = STATE_SHOWING;

unsigned long counter0 = 0;
unsigned long pressing_start;
char pressing = 0;
int showing = 0;
int timespeak = 0;
int isSpeak = 1;
void show_empty();
void show_number(int n);
void show_clock(int n);

void init() {
	TMOD = 0x11;	 // 16 bit
	IP = 0x1A;	   // interrupt priorities

	EX0 = 1;
	EX1 = 1;
	EA = 1;

	ET0 = 1;
	TR0 = 1;

	ET1 = 1;
	TR1 = 0;
}
int current_time() {
	return (counter0 - start_counting_time) / 20;
}

int current_dry_time() {
	return (counter0 - start_too_dry_time) / 20;
}

void start_beep() {
	TR1 = 0;
	// 2ms
	TH1 = 0xF8;
	TL1 = 0x30;
	TR1 = 1;
}

void too_dry() {
	start_too_dry_time = counter0;
	is_too_dry = 1;
	P2_3 = 0;
	enFlicker++;
	start_beep();
}

void short_pressing() {	
	if (state == STATE_SHOWING) {
		//P2_0 = ~P2_0;
		if(showing==0){
			showing = 1;
			P2_0 = 0;
		}
		else{
			showing = 0;
			P2_0=1;
		}
	}
	else if (state == STATE_SETTING) {
		max_time = (max_time) % 9 + 1;
	}
}

void long_pressing() {
	//P2_1 = ~P2_1;
	if (state == STATE_SHOWING)	{
		P2_1=0;
		state = STATE_SETTING;
		enFlicker++;
	}
	else if (state == STATE_SETTING) {
		P2_1=1;
		state = STATE_SHOWING;
		enFlicker--;
	}
}

void timer0() interrupt 1 {
	// 50ms
	TH0 = 0x3c;
	TL0 = 0xb0;
	counter0++;		 		// cu 50ms thi tang len 1

	if (counter0 % 10 == 0)			// nhap nhay
		flicker = ~flicker & 0x1;

	if (!is_too_dry && current_time() / 60 >= max_time)
		too_dry();
}

void timer1() interrupt 3 {
	if (pressing) {
		TR1 = 0;
		pressing = 0;	
		// 500ms
		if (counter0 - pressing_start >= 20)
			long_pressing();
		else
			short_pressing();

		if (is_too_dry)
			start_beep();
	}
	else {			 // phat loa
		TH1 = 0xF8;
		TL1 = 0x30;
		
	   	if(isSpeak ==1){
			P1_5 = ~P1_5; 
		}

		timespeak +=1;
		if (timespeak == 100){
			isSpeak = !isSpeak;
			timespeak = 0;	
		}
	}
}

void external0() interrupt 0 {
	if (!pressing) {
		pressing = 1;
		pressing_start = counter0;
	}			 	


	// 2ms
	TR1 = 0;
	TH1 = 0xF8;
	TL1 = 0x30;
	TR1 = 1;
}
void external1() interrupt 2 {		   // sensor tuoi nuoc
	start_counting_time = counter0;	   // quay lai trang thai dau
	enFlicker=0;
	is_too_dry = 0;
	TR1 = 0;
	//P2_0 = 1; //trang thai hien thi t :1 hay T :0
	//P2_1 = 1; // trang thai SHOWING:1 hay SETTING:0
	P2_3=1;	  // trang thái too_dry: 0
	P2_4 = 0;
	P2_4 = 1;
}
void main() {
	P2 = 0xff;
	init();
   	
	while (1) {
		if (state == STATE_SHOWING) {
			if (!pressing)
				if (!is_too_dry)
					if(showing==0){ 
						show_clock(current_time());
					}
					else{
						show_number(max_time);
					}

				else
					show_clock(current_time());
			else
				show_empty();
		}
		else if (state == STATE_SETTING) {
			if (!pressing)
				show_number(max_time);
			else
				show_empty();
		}
	}
}

void show_empty() {
	P1_0 = 0;
	P0 = 0xff;
	P1_0 = 1;

	P1_1 = 0;
	P0 = 0xff;
	P1_1 = 1;

	P1_2 = 0;
	P0 = 0xff;
	P1_2 = 1;

	P1_3 = 0;
	P0 = 0xff;
	P1_3 = 1;
}

void __show_number(int led, int n, int dot) 
{
	int i;

	switch (led) {
	case 0:
		P1_0 = 0;
		break;
	case 1:
		P1_1 = 0;
		break;
	case 2:
		P1_2 = 0;
		break;
	case 3:
		P1_3 = 0;
		break;
	}

	switch (n) {
	case 0:
		P0 = ~0x3f;
		break;
	case 1:
		P0 = ~0x06;
		break;
	case 2:
		P0 = ~0x5b;
		break;
	case 3:
		P0 = ~0x4f;
		break;
	case 4:
		P0 = ~0x66;
		break;
	case 5:
		P0 = ~0x6d;
		break;
	case 6:
		P0 = ~0x7d;
		break;
	case 7:
		P0 = ~0x07;
		break;
	case 8:
		P0 = ~0x7f;
		break;
	case 9:
		P0 = ~0x6f;
		break;
	}

	if (dot) 
		P0_7 = 0;

	// Sleep
	i = 400;
	while (i--);

	switch (led) {
	case 0:
		P1_0 = 1;
		break;
	case 1:
		P1_1 = 1;
		break;
	case 2:
		P1_2 = 1;
		break;
	case 3:
		P1_3 = 1;
		break;
	}
}

void show_number(int n) {							
	int n3, n2, n1, n0;

	if (enFlicker && flicker) {
		show_empty();	
		return;
	}
	
	n3 = n % 10; n /= 10;
	n2 = n % 10; n /= 10;
	n1 = n % 10; n /= 10;
	n0 = n;

	__show_number(0, n0, 0);
	__show_number(1, n1, 0);
	__show_number(2, n2, 0);
	__show_number(3, n3, 0);
}
void show_clock(int n) {							
	int n3, n2, n1, n0;
	int minute,second;
	if (enFlicker && flicker) {
		show_empty();	
		return;
	}
	minute = n/60;
	second = n%60;
	n1 = minute % 10; minute /= 10;
	n0 = minute;
	n3 = second % 10; second /= 10;
	n2 = second;

	__show_number(0, n0, 0);
	__show_number(1, n1, 1);
	__show_number(2, n2, 0);
	__show_number(3, n3, 0);
}