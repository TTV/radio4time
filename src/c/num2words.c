#include "num2words.h"
#include "string.h"

static const char* const ONES[] = {
	"zero",
	"one",
	"two",
	"three",
	"four",
	"five",
	"six",
	"seven",
	"eight",
	"nine"
};

static const char* const TEENS[] ={
	"",
	"eleven",
	"twelve",
	"thirteen",
	"fourteen",
	"fifteen",
	"sixteen",
	"seventeen",
	"eighteen",
	"nineteen"
};

static const char* const TENS[] = {
	"",
	"ten",
	"twenty",
	"thirty",
	"forty",
	"fifty"
};

static const char* STR_EXACTLY = "exactly";
static const char* STR_ALMOST = "almost";
static const char* STR_APPROACHING = "coming up to";
static const char* STR_JUSTGONE = "just gone";
static const char* STR_AFTER = "after";
static const char* STR_QUARTER = "quarter";
static const char* STR_HALF = "half";
static const char* STR_TO = "to";
static const char* STR_PAST = "past";
static const char* STR_OCLOCK = "O'Clock";

static size_t append_string(char* buffer, const size_t length, const char* str) {
	strncat(buffer, str, length);
	size_t written = strlen(str);
	return (length > written) ? written : length;
}

static size_t append_number(char* buffer, const size_t length, int num) {
	int tens_val = num / 10 % 10;
	int ones_val = num % 10;

	size_t len = 0;

	if (tens_val > 0) {
	if (tens_val == 1 && num != 10)
		return append_string(buffer, length, TEENS[ones_val]);
	len += append_string(buffer, length, TENS[tens_val]);
	if (ones_val > 0)
		len += append_string(buffer, length, " ");
	}

	if (ones_val > 0 || num == 0)
	len += append_string(buffer, length, ONES[ones_val]);

	return len;
}

void fuzzy_time_to_words(int hours, int minutes, char* header, size_t hLength, char* words, size_t length ) {
	size_t hRemaining = hLength;
	memset(header, 0, hLength);
	
	size_t remaining = length;
	memset(words, 0, length);

	int fuzzy_hours = hours;
	int fuzzy_minutes = ( minutes / 5 ) * 5;
	int fuzzy_minutes_offset = minutes % 5;

	switch (fuzzy_minutes_offset) {
		case 0:
			hRemaining -= append_string(header, hRemaining, STR_EXACTLY);
			break;
		case 1:
			hRemaining -= append_string(header, hRemaining, STR_JUSTGONE);
			break;
		case 2:
			hRemaining -= append_string(header, hRemaining, STR_AFTER);
			break;
		case 3:
			hRemaining -= append_string(header, hRemaining, STR_APPROACHING);
			fuzzy_minutes += 5;
			break;
		case 4:
			hRemaining -= append_string(header, hRemaining, STR_ALMOST);
			fuzzy_minutes += 5;
			break;
	}

	int addOclock = 0;
	if ((fuzzy_minutes > 0) && (fuzzy_minutes < 60)){
		if ((fuzzy_minutes == 15) || (fuzzy_minutes == 45))
			remaining -= append_string(words, remaining, STR_QUARTER);
		else if (fuzzy_minutes == 30)
			remaining -= append_string(words, remaining, STR_HALF);
		else {
			if (fuzzy_minutes >= 35)
				remaining -= append_number(words, remaining, 60-fuzzy_minutes);
			else
				remaining -= append_number(words, remaining, fuzzy_minutes);
		}
		remaining -= append_string(words, remaining, " ");
		if (fuzzy_minutes <= 30)
			remaining -= append_string(words, remaining, STR_PAST);
		else
			remaining -= append_string(words, remaining, STR_TO);
		remaining -= append_string(words, remaining, "\n");
	} else
		addOclock = 1;

	if (fuzzy_minutes >= 35)
		fuzzy_hours++;

	fuzzy_hours = fuzzy_hours % 12;
	if (fuzzy_hours == 0)
		fuzzy_hours = 12;
	remaining -= append_number(words, remaining, fuzzy_hours);
	
	if (addOclock) {
		remaining -= append_string(words, remaining, "\n");
		remaining -= append_string(words, remaining, STR_OCLOCK);
	}
}
