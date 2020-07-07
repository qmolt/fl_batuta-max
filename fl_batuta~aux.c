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

/* text ---------------------------------------------------------------------- */
void my_setparse(long *ac, t_atom **av, char *line)
{
	long m_ac = 0;
	t_atom *m_av = NULL;
	t_max_err err;
	err = atom_setparse(&m_ac, &m_av, line);
	*ac = m_ac;
	atom_getatom_array(m_ac, m_av, *ac, *av);
}
void my_gettext(long ac, t_atom *av, long *text_len, char **text, long flag)
{
	char *m_texto = (char *)sysmem_newptr(LARGO_MAX_LINEA * sizeof(char));
	long m_texto_len = 0;

	atom_gettext(ac, av, &m_texto_len, &m_texto, flag);
	*text_len = m_texto_len;
	strncpy_zero(*text, m_texto, m_texto_len);
	sysmem_freeptr(m_texto);
}
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

/* math ---------------------------------------------------------------------- */
float parse_curve(float curva)
{
	if (curva < CURVE_MIN) { curva = CURVE_MIN; }
	else if (curva > CURVE_MAX) { curva = CURVE_MAX; }

	if (curva > 0.0) { return (float)(1.0 / (1.0 - curva)); }
	else { return (float)(curva + 1.0); }
}
float msbeat_to_bpm(float ms_beat)
{
	float bpm = 60000 / ms_beat;
	return bpm;
}

/* comparison (menor) -------------------------------------------------------- */
long cifra_compasmenor(fl_cifra *a, fl_cifra *b)
{
	fl_cifra *c1 = (fl_cifra *)a;
	fl_cifra *c2 = (fl_cifra *)b;

	if (c1->n_bar < c2->n_bar) { return 1; }
	else { return 0; }
}
long goto_compasmenor(fl_goto *a, fl_goto *b)
{
	fl_goto *c1 = (fl_goto *)a;
	fl_goto *c2 = (fl_goto *)b;

	if (c1->n_bar < c2->n_bar) { return 1; }
	else { return 0; }
}
long tempo_compasmenor(fl_tempo *a, fl_tempo *b)
{
	fl_tempo *c1 = (fl_tempo *)a;
	fl_tempo *c2 = (fl_tempo *)b;

	if (c1->n_bar < c2->n_bar) { return 1; }
	else { return 0; }
}
long nota_iniciomenor(fl_nota *a, fl_nota *b)
{
	fl_nota *c1 = (fl_nota *)a;
	fl_nota *c2 = (fl_nota *)b;

	if (c1->b_inicio < c2->b_inicio) { return 1; }
	else { return 0; }
}

/* comparison (equal) -------------------------------------------------------- */
long cifra_mismocompas(fl_cifra *a, fl_cifra *b)
{
	fl_cifra *c1 = (fl_cifra *)a;
	fl_cifra *c2 = (fl_cifra *)b;

	if (c1->n_bar == c2->n_bar) { return 1; }
	else { return 0; }
}
long goto_mismocompas(fl_goto *a, fl_goto *b)
{
	fl_goto *c1 = (fl_goto *)a;
	fl_goto *c2 = (fl_goto *)b;

	if (c1->n_bar == c2->n_bar) { return 1; }
	else { return 0; }
}
long tempo_mismocompas(fl_tempo *a, fl_tempo *b)
{
	fl_tempo *c1 = (fl_tempo *)a;
	fl_tempo *c2 = (fl_tempo *)b;

	if (c1->n_bar == c2->n_bar) { return 1; }
	else { return 0; }
}