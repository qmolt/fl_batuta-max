#include "fl_batuta~.h"

/* aux ui -------------------------------------------------------------------- */
t_jrgb hsltorgb(double h, double s, double l) {
	t_jrgb color_rgb;
	double r, g, b, p, q, hue, sat, lig;
	hue = h;
	sat = s;
	lig = l;

	if (sat == 0) {
		color_rgb.red = color_rgb.green = color_rgb.blue = lig; // achromatic
	}
	else {
		q = lig < 0.5 ? lig * (1. + sat) : lig + sat - lig * sat;
		p = 2. * lig - q;
		r = huetorgb(p, q, hue + 1. / 3.);
		g = huetorgb(p, q, hue);
		b = huetorgb(p, q, hue - 1. / 3.);

		color_rgb.red = r;
		color_rgb.green = g;
		color_rgb.blue = b;
	}
	return color_rgb;
}
t_double huetorgb(double p, double q, double t) {
	if (t < 0.) t += 1.;
	if (t > 1.) t -= 1.;
	if (t < 1. / 6.) return p + (q - p) * 6. * t;
	if (t < 1. / 2.) return q;
	if (t < 2. / 3.) return p + (q - p) * (2. / 3. - t) * 6.;
	return p;
}

//text ---------------------------------------------------------------------- 
/*
int getlinea(char *dest, char *orig, int lim)
{
	int i;

	for (i = 0; (i < lim) && (orig[i] != '\n') && (orig + i != NULL); i++) {
		if (orig[i] != '\r') {
			dest[i] = orig[i];
		}
		else {
			dest[i] = ' ';
		}
	}
	if (orig[i] == '\n' || orig[i] == EOF || orig[i] == '\0' || orig + i == NULL) {
		dest[i] = '\0';
		++i;
	}

	return i;
}
*/

/* math ---------------------------------------------------------------------- */
float parse_curve(float curva)
{
	curva = MIN(MAX(curva, CURVE_MIN), CURVE_MAX);
	if (curva > 0.0) { return (float)(1.0 / (1.0 - curva)); } //c in (0,1)-> ret (1,0)
	else { return (float)(curva + 1.0); }//c in (-1,0]-> ret [0,1]
}
float msbeat_to_bpm(float ms_beat)
{
	float bpm = 60000 / ms_beat;
	return bpm;
}

/* comparison (previous) -------------------------------------------------------- */
long signature_prevbar(fl_tsign *a, fl_tsign *b)
{
	fl_tsign *c1 = (fl_tsign *)a;
	fl_tsign *c2 = (fl_tsign *)b;

	if (c1->n_bar < c2->n_bar) { return 1; }
	else { return 0; }
}
long goto_prevbar(fl_goto *a, fl_goto *b)
{
	fl_goto *c1 = (fl_goto *)a;
	fl_goto *c2 = (fl_goto *)b;

	if (c1->n_bar < c2->n_bar) { return 1; }
	else { return 0; }
}
long tempo_prevbar(fl_tempo *a, fl_tempo *b)
{
	fl_tempo *c1 = (fl_tempo *)a;
	fl_tempo *c2 = (fl_tempo *)b;

	if (c1->n_bar < c2->n_bar) { return 1; }
	else { return 0; }
}
long note_prevstart(fl_note *a, fl_note *b)
{
	fl_note *c1 = (fl_note *)a;
	fl_note *c2 = (fl_note *)b;

	if (c1->b_inicio < c2->b_inicio) { return 1; }
	else { return 0; }
}

/* comparison (equal) -------------------------------------------------------- */
long signature_samebar(fl_tsign *a, fl_tsign *b)
{
	fl_tsign *c1 = (fl_tsign *)a;
	fl_tsign *c2 = (fl_tsign *)b;

	if (c1->n_bar == c2->n_bar) { return 1; }
	else { return 0; }
}
long goto_samebar(fl_goto *a, fl_goto *b)
{
	fl_goto *c1 = (fl_goto *)a;
	fl_goto *c2 = (fl_goto *)b;

	if (c1->n_bar == c2->n_bar) { return 1; }
	else { return 0; }
}
long tempo_samebar(fl_tempo *a, fl_tempo *b)
{
	fl_tempo *c1 = (fl_tempo *)a;
	fl_tempo *c2 = (fl_tempo *)b;

	if (c1->n_bar == c2->n_bar) { return 1; }
	else { return 0; }
}