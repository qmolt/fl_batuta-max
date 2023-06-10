#include "flbatuta~.h"

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
	
	clock_delay(x->cursor_clock, 0);
}

void fl_batuta_perform64(t_fl_batuta *x, t_object *dsp64, double **inputs, long numinputs, double **outputs, long numoutputs, long vectorsize, long flags, void *userparam)
{
	double *output_beat = outputs[0];
	double *output_bar = outputs[1];
	double *output_tempo = outputs[2];
	long n = vectorsize;

	double sr = x->sr;
	char isplaying = x->isplaying;
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
	short task_out_bar = x->task_out_bar;
	short task_new_idx = x->task_new_idx;
	short task_end_flag = 0;

	long total_tempos = x->total_tempos;
	fl_tempo **htempo = x->tempos;
	long index_tempo = x->index_tempo;
	short task_tempo = x->task_tempo;
	long cont_tempo = x->cont_tempo;
	long durac_dtempo = x->durac_dtempo;
	float old_msbeat = x->old_msbeat;
	float new_msbeat = x->new_msbeat;
	float curva_dtempo = x->curva_dtempo;
	long delay_dtempo = x->delay_dtempo;

	fl_tsign **hcifra = x->tsigns;
	long total_tsigns = x->total_tsigns;
	long index_cifra = x->index_cifra;
	fl_goto **hgoto = x->gotos;
	long total_gotos = x->total_gotos;
	long index_goto = x->index_goto;
	fl_note **hnotes_out = x->notes_out;
	long index_nota = x->index_nota;
	fl_bar **hbars = x->bars;
	fl_bar *pbar;
	fl_note **hnota;
	long cont_notas = 0;
	long index_compas = x->index_compas;

	double cursor = x->fasor_cursor;

	float xi;
	double total_samps_ant;

	total_samps = (long)(negras * ms_beat * sr * 0.001);
	total_beat = (long)(total_samps / negras);

	while (n--) {
		
		if (x->isplaying) {
			//force n bar
			if (next_bar_dirty) {
				n_bar = MIN(next_bar, total_bars - 1);
				index_tempo = index_cifra = index_goto = 0;
				samps_bar = 0;
				samps_beat = 0;

				next_bar_dirty = 0;
				task_tempo = 0;
			}

			//update bar
			if (samps_bar++ > total_samps) {
				samps_bar = 0;
				n_bar++;

				//tasks
				task_new_idx = 1;
				task_out_bar = 1;
				if (n_bar >= total_bars) {
					task_end_flag = 1;
					x->isplaying = 0;
				}
			}
		}

		if(x->isplaying){
			if (task_new_idx) {
				//goto
				while (index_goto < total_gotos) {
					if (hgoto[index_goto]->n_bar < n_bar) {
						;
					}
					else if (hgoto[index_goto]->n_bar == n_bar) {
						if ((hgoto[index_goto]->cont_rep)++ < hgoto[index_goto]->total_rep) {
							n_bar = MIN(hgoto[index_goto]->to_bar, total_bars - 1);
							index_nota = index_tempo = index_cifra = index_goto = 0;
						}
						break;
					}
					else {
						break;
					}
					index_goto++;
				}
				//time signature
				while (index_cifra < total_tsigns) {
					if (hcifra[index_cifra]->n_bar < n_bar) {
						negras = hcifra[index_cifra]->beats;
					}
					else if (hcifra[index_cifra]->n_bar == n_bar) {
						negras = hcifra[index_cifra]->beats;

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
					index_cifra++;
				}
				x->negras = negras;

				//tempo
				while (index_tempo < total_tempos && task_tempo == TT_FINDTEMPO) {
					if (htempo[index_tempo]->n_bar < n_bar) {
						ms_beat = htempo[index_tempo]->ms_beat;
					}
					else if (htempo[index_tempo]->n_bar == n_bar) {
						delay_dtempo = (long)((htempo[index_tempo]->ms_inicio) * sr * 0.001);
						new_msbeat = htempo[index_tempo]->ms_beat;
						old_msbeat = ms_beat;
						task_tempo = TT_DELAYDELTA;
						break;
					}
					else {
						break;
					}
					index_tempo++;
				}

				task_new_idx = 0;
			}

			if (task_tempo == TT_DELAYDELTA) {
				if (--delay_dtempo < 0) {
					durac_dtempo = (long)(htempo[index_tempo]->ms_durvar * sr * 0.001);
					curva_dtempo = htempo[index_tempo]->powval;
					cont_tempo = 0;
					task_tempo = TT_DELTATEMPO;
				}
			}
			if (task_tempo == TT_DELTATEMPO) {
				if (cont_tempo++ <= durac_dtempo) {
					xi = (float)(cont_tempo / (float)durac_dtempo);
					ms_beat = old_msbeat + (float)pow(xi, curva_dtempo) * (new_msbeat - old_msbeat);

					total_samps_ant = total_samps;

					total_samps = (long)(negras * ms_beat * sr * 0.001);
					total_beat = (total_samps / (long)ceil(negras));

					samps_bar *= ((double)total_samps / total_samps_ant);
				}
				else {
					task_tempo = 0;
					index_tempo++;
				}
			}

			//notes
			if (index_compas != n_bar) {
				index_compas = n_bar;
				index_nota = 0;
			}
			pbar = hbars[index_compas];
			if (index_nota < pbar->total_notas) {
				cont_notas = 0;
				hnota = pbar->hdl_nota;
				while (samps_bar >= (double)hnota[index_nota]->b_inicio * total_beat) {
					hnotes_out[cont_notas++] = hnota[index_nota++];
					x->total_notes_out = cont_notas;
					if (index_nota >= hbars[index_compas]->total_notas) { break; }
				}
				if (cont_notas > 0) { clock_delay(x->outmes_clock, 0); }
			}
		}
		else { 
			samps_bar = samps_beat = 0; 
		}

		if (task_out_bar) { 
			clock_delay(x->outbar_clock, 0); 
			task_out_bar = 0;
		}
		if (task_end_flag) { 
			clock_delay(x->bang_clock, 0);
			task_end_flag = 0;
		}

		samps_beat = (long)samps_bar % (long)(total_samps / negras);
		*output_beat++ = (double)samps_beat / (double)total_beat;
		cursor = *output_bar++ = samps_bar / (double)total_samps;
		*output_tempo++ = (double)ms_beat;
	}

	/* update state variables */
	x->fasor_cursor = cursor;
	x->samps_bar = samps_bar;
	x->total_samps = total_samps;
	x->samps_beat = samps_beat;
	x->total_beat = total_beat;
	x->n_bar = n_bar;
	x->ms_beat = ms_beat;
	x->task_tempo = task_tempo;
	x->index_compas = index_compas;
	x->index_tempo = index_tempo;
	x->index_cifra = index_cifra;
	x->index_goto = index_goto;
	x->index_nota = index_nota;
	x->next_bar_dirty = next_bar_dirty;
	x->old_msbeat = old_msbeat;	//for tempo variation
	x->new_msbeat = new_msbeat;
	x->task_tempo = task_tempo;
	x->cont_tempo = cont_tempo;
	x->durac_dtempo = durac_dtempo;
	x->curva_dtempo = curva_dtempo;
	x->delay_dtempo = delay_dtempo;
}
