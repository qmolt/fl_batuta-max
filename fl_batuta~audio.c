#include "fl_batuta~.h"

void fl_batuta_dsp64(t_fl_batuta *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	x->fasor_cursor = 0.;
	x->samps_bar = 0;
	x->total_samps = 500;
	x->samps_beat = 0;
	x->total_beat = 500;

	if (x->sr != samplerate) {
		x->sr = samplerate;
	}

	object_method(dsp64, gensym("dsp_add64"), x, fl_batuta_perform64, 0, NULL);
	x->startclock = true;
}

void fl_batuta_perform64(t_fl_batuta *x, t_object *dsp64, double **inputs, long numinputs, double **outputs, long numoutputs, long vectorsize, long flags, void *userparam)
{
	/* Copy signal pointers and signal vector size */
	double *output_beat = outputs[0];
	double *output_bar = outputs[1];
	double *output_tempo = outputs[2];
	long n = vectorsize;

	/* load state variables */
	double sr = x->sr;
	short onoff = x->onoff;
	double samps_bar = x->samps_bar;
	long total_samps = x->total_samps;
	long samps_beat = x->samps_beat;
	long total_beat = x->total_beat;
	float ms_beat = x->ms_beat;
	float negras = x->negras;
	long n_bar = x->n_bar;
	long total_bars = x->total_bars;
	short next_bar_dirty = x->next_bar_dirty;
	long next_bar = x->next_bar;

	long total_tempos = x->total_tempos;
	fl_tempo **htempo = x->tempos;
	long index_tempo = x->index_tempo;
	short dtempo_busy = x->dtempo_busy;
	long cont_tempo = x->cont_tempo;
	long durac_dtempo = x->durac_dtempo;
	float old_msbeat = x->old_msbeat;
	float new_msbeat = x->new_msbeat;
	float curva_dtempo = x->curva_dtempo;
	short type_dtempo = x->type_dtempo;
	long delay_dtempo = x->delay_dtempo;

	fl_cifra **hcifra = x->cifras;
	long total_cifras = x->total_cifras;
	long index_cifra = x->index_cifra;
	fl_goto **hgoto = x->gotos;
	long total_gotos = x->total_gotos;
	long index_goto = x->index_goto;
	fl_nota **hnotas_out = x->notas_out;
	long index_nota = x->index_nota;
	fl_bar **hbars = x->compases;
	fl_bar *pbar;
	fl_nota **hnota;
	long cont_notas = 0;
	long index_compas = x->index_compas;

	double cursor = x->fasor_cursor;

	/* declare variables and make calculations */
	float xi;
	double total_samps_ant;

	total_samps = (long)(negras * ms_beat * sr * 0.001);
	total_beat = (long)(total_samps / negras);

	/* Perform the DSP loop */
	while (n--) {
		if (onoff) {
			//reiniciar contador
			if (samps_bar++ > total_samps || n_bar < 0 || n_bar >= total_bars) {
				samps_bar = 0;

				//inicio
				x->n_bar = n_bar = (n_bar < 0) ? 0 : x->n_bar + 1;
				clock_delay(x->outbar_clock, 0);

				//final
				if (n_bar >= total_bars) {
					clock_delay(x->bang_clock, 0);
					x->onoff = 0;
					n_bar = 0;
				}

				//goto
				while (index_goto < total_gotos) {
					if (hgoto[index_goto]->n_bar < n_bar) {
						index_goto++;
					}
					else if (hgoto[index_goto]->n_bar == n_bar) {
						if ((hgoto[index_goto]->cont_rep)++ < hgoto[index_goto]->total_rep) {
							n_bar = hgoto[index_goto]->to_bar;
							index_nota = index_tempo = index_cifra = index_goto = 0;
						}
						break;
					}
					else { break; }
				}

				//cambiar a proximo bar si es que hay
				if (next_bar_dirty) {
					n_bar = next_bar;
					index_tempo = index_cifra = index_goto = 0;
					next_bar_dirty = 0;
					dtempo_busy = 0;
				}

				//cifra
				while (index_cifra < total_cifras) {
					if (hcifra[index_cifra]->n_bar < n_bar) {
						x->negras = negras = hcifra[index_cifra]->negras;
						index_cifra++;
					}
					else if (hcifra[index_cifra]->n_bar == n_bar) {
						x->negras = negras = hcifra[index_cifra]->negras;

						total_samps_ant = total_samps;

						total_samps = (long)(negras * ms_beat * sr * 0.001);
						total_beat = (total_samps / (long)ceil(negras));

						samps_bar *= ((double)total_samps / total_samps_ant);

						clock_delay(x->outcifra_clock, 0);
						break;
					}
					else {
						break;
					}
				}

				//tempo
				while (index_tempo < total_tempos && dtempo_busy == 0) {
					if (htempo[index_tempo]->n_bar < n_bar) {
						ms_beat = htempo[index_tempo]->ms_beat;
						index_tempo++;
					}
					else if (htempo[index_tempo]->n_bar == n_bar) {
						delay_dtempo = (long)((htempo[index_tempo]->ms_inicio) * sr * 0.001);
						new_msbeat = htempo[index_tempo]->ms_beat;
						old_msbeat = ms_beat;
						type_dtempo = htempo[index_tempo]->type;
						dtempo_busy = 1;
						break;
					}
					else {
						dtempo_busy = 1;
						break;
					}
				}
			}

			//var tempo
			if (dtempo_busy == 1) {
				if (delay_dtempo <= 0) {
					if (type_dtempo == 0) {
						ms_beat = new_msbeat;
						dtempo_busy = 0;

						total_samps_ant = total_samps;

						total_samps = (long)(negras * ms_beat * sr * 0.001);
						total_beat = (total_samps / (long)ceil(negras));

						samps_bar *= ((double)total_samps / total_samps_ant);
					}
					else if (type_dtempo == 1) {
						durac_dtempo = (long)(htempo[index_tempo]->ms_durvar * sr * 0.001);
						curva_dtempo = 1.0;
						cont_tempo = 0;
						dtempo_busy = 2;
					}
					else {
						durac_dtempo = (long)(htempo[index_tempo]->ms_durvar * sr * 0.001);
						curva_dtempo = htempo[index_tempo]->curva;
						cont_tempo = 0;
						dtempo_busy = 2;
					}
				}
				else { delay_dtempo--; }
			}
			if (dtempo_busy == 2) {
				if (cont_tempo++ <= durac_dtempo) {
					xi = (float)(cont_tempo / (float)durac_dtempo);
					ms_beat = old_msbeat + (float)pow(xi, curva_dtempo) * (new_msbeat - old_msbeat);

					total_samps_ant = total_samps;

					total_samps = (long)(negras * ms_beat * sr * 0.001);
					total_beat = (total_samps / (long)ceil(negras));

					samps_bar *= ((double)total_samps / total_samps_ant);
				}
				else {
					dtempo_busy = 0;
					index_tempo++;
				}
			}

			//notas
			if (index_compas != n_bar) {
				index_compas = n_bar;
				index_nota = 0;
			}
			pbar = hbars[index_compas];
			if (index_nota < pbar->total_notas) {
				cont_notas = 0;
				hnota = pbar->hdl_nota;
				while (samps_bar >= (double)hnota[index_nota]->b_inicio * total_beat) {
					hnotas_out[cont_notas++] = hnota[index_nota++];
					x->total_notas_out = cont_notas;
					if (index_nota >= hbars[index_compas]->total_notas) { break; }
				}
				if (cont_notas > 0) { clock_delay(x->outmes_clock, 0); }
			}
		}
		samps_beat = (long)samps_bar % (long)(total_samps / negras);
		*output_beat++ = (double)samps_beat / (double)total_beat;
		cursor = *output_bar++ = samps_bar / (double)total_samps;
		*output_tempo++ = (double)ms_beat;
	}

	if (x->startclock) {
		x->startclock = 0;
		clock_delay(x->cursor_clock, 0);
	}

	/* update state variables */
	x->fasor_cursor = cursor;
	x->samps_bar = samps_bar;
	x->total_samps = total_samps;
	x->samps_beat = samps_beat;
	x->total_beat = total_beat;
	x->n_bar = n_bar;
	x->ms_beat = ms_beat;
	x->negras = negras;
	x->dtempo_busy = dtempo_busy;
	x->index_compas = index_compas;
	x->index_tempo = index_tempo;
	x->index_cifra = index_cifra;
	x->index_goto = index_goto;
	x->index_nota = index_nota;
	x->next_bar_dirty = next_bar_dirty;
	x->old_msbeat = old_msbeat;	//cambio de tempo
	x->new_msbeat = new_msbeat;
	x->dtempo_busy = dtempo_busy;
	x->cont_tempo = cont_tempo;
	x->durac_dtempo = durac_dtempo;
	x->curva_dtempo = curva_dtempo;
	x->type_dtempo = type_dtempo;
	x->delay_dtempo = delay_dtempo;
}
