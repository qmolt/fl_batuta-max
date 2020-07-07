#include "fl_batuta~.h"

void fl_batuta_actualizar(t_fl_batuta *x)
{
	if (x->onoff) { object_error((t_object *)x, "no se puede actualizar en modo play"); return; }

	fl_batuta_actualizar_notas(x);
	fl_batuta_actualizar_tempos(x);
	fl_batuta_actualizar_cifras(x);
	fl_batuta_actualizar_gotos(x);
	//object_post((t_object *)x,"actualizado");
}

void fl_batuta_actualizar_notas(t_fl_batuta *x)
{
	fl_bar *pbar;
	long total_compases;
	long total_notas;

	x->total_bars = total_compases = (long)linklist_getsize(x->l_bars);
	if (total_compases == 0) {
		x->compases = (fl_bar **)sysmem_resizeptr(x->compases, sizeof(fl_bar *));
		x->compases[0] = x->pbar_error;
	}
	else {
		x->compases = (fl_bar **)sysmem_resizeptr(x->compases, total_compases * sizeof(fl_bar *));
	}

	for (int i = 0; i < total_compases; i++) {
		x->compases[i] = pbar = linklist_getindex(x->l_bars, i);
		pbar->total_notas = total_notas = (long)linklist_getsize(pbar->notas);

		linklist_sort(pbar->notas, nota_iniciomenor);

		pbar->hdl_nota = (fl_nota **)sysmem_resizeptr(pbar->hdl_nota, total_notas * sizeof(fl_nota *));

		if (total_notas == 0) { continue; }

		//if (pbar->hdl_nota == NULL) { //no debiera ocurrir
		//	pbar->hdl_nota = (fl_nota **)sysmem_newptr(total_notas * sizeof(fl_nota *));
		//	if (pbar->hdl_nota == NULL) { continue; }
		//}
		//else {
		pbar->hdl_nota = (fl_nota **)sysmem_resizeptr(pbar->hdl_nota, total_notas * sizeof(fl_nota *));
		//}

		for (int j = 0; j < total_notas; j++) {
			pbar->hdl_nota[j] = linklist_getindex(pbar->notas, j);
		}
	}

	x->nota_dirty = 0;
	x->index_nota = 0;

	fl_batuta_actualizar_uibar(x);
}

void fl_batuta_actualizar_tempos(t_fl_batuta *x)
{
	long total_tempos = (long)linklist_getsize(x->l_tempos);
	x->tempos = (fl_tempo **)sysmem_resizeptr(x->tempos, total_tempos * sizeof(fl_tempo *));
	for (int i = 0; i < total_tempos; i++) {
		x->tempos[i] = linklist_getindex(x->l_tempos, i);
	}
	x->total_tempos = total_tempos;
	x->index_tempo = 0;
	x->ms_beat = x->tempos[0]->ms_beat;
	x->tempo_dirty = 0;
	fl_batuta_actualizar_uibar(x);
}

void fl_batuta_actualizar_cifras(t_fl_batuta *x)
{
	long total_cifras = (long)linklist_getsize(x->l_cifras);
	x->cifras = (fl_cifra **)sysmem_resizeptr(x->cifras, total_cifras * sizeof(fl_cifra *));
	for (int i = 0; i < total_cifras; i++) {
		x->cifras[i] = linklist_getindex(x->l_cifras, i);
	}
	x->total_cifras = total_cifras;
	x->index_cifra = 0;
	x->negras = x->cifras[0]->negras;
	x->cifra_dirty = 0;
	fl_batuta_actualizar_uibar(x);
}

void fl_batuta_actualizar_gotos(t_fl_batuta *x)
{
	long total_gotos = (long)linklist_getsize(x->l_gotos);
	x->gotos = (fl_goto **)sysmem_resizeptr(x->gotos, total_gotos * sizeof(fl_goto *));
	for (int i = 0; i < total_gotos; i++) {
		x->gotos[i] = linklist_getindex(x->l_gotos, i);
		x->gotos[i]->cont_rep = 0;
	}
	x->total_gotos = total_gotos;

	x->index_goto = 0;
	fl_batuta_actualizar_uibar(x);
}

void fl_batuta_actualizar_uibar(t_fl_batuta *x)
{
	long index_cifra;
	long index_tempo;
	long index_goto;
	fl_bar *pbar;
	long total_compases, total_notas;
	long total_cifras = (long)linklist_getsize(x->l_cifras);
	long total_tempos = (long)linklist_getsize(x->l_tempos);
	long total_gotos = (long)linklist_getsize(x->l_gotos);
	fl_cifra *pcifra;
	fl_tempo *ptempo;
	fl_goto *pgoto;
	float cifra_bar;
	short is_bar_error = 0;

	x->jn_total_bars = total_compases = (long)linklist_getsize(x->l_bars);

	x->jn_cfra_ini = (total_cifras == 0) ? 0 : 1;
	x->jn_tmpo_ini = (total_tempos == 0) ? 0 : 1;
	x->jn_nota_ini = (total_compases == 0) ? 0 : 1;

	for (int i = 0; i < total_compases; i++) {

		pbar = x->compases[i];

		//encontrar cifra
		index_cifra = 0;
		cifra_bar = -1;
		while (index_cifra < total_cifras) {
			pcifra = linklist_getindex(x->l_cifras, index_cifra);
			if (pcifra->n_bar <= i) {
				cifra_bar = pcifra->negras;
			}
			index_cifra++;
		}
		pbar->cifra_ui = cifra_bar;

		//encontrar tempo
		pbar->tempo_ui.type = -1;
		pbar->tempo_ui.ms_beat = pbar->tempo_ui.ms_durvar = pbar->tempo_ui.curva = -1.;
		pbar->tempo_ui.n_bar = -1;

		index_tempo = 0;
		while (index_tempo < total_tempos) {
			ptempo = linklist_getindex(x->l_tempos, index_tempo);
			if (ptempo->n_bar <= i) {
				pbar->tempo_ui.type = ptempo->type;
				pbar->tempo_ui.ms_beat = ptempo->ms_beat;
				pbar->tempo_ui.ms_durvar = ptempo->ms_durvar;
				pbar->tempo_ui.curva = ptempo->curva;
				pbar->tempo_ui.ms_inicio = ptempo->ms_inicio;
				pbar->tempo_ui.n_bar = ptempo->n_bar;
			}
			index_tempo++;
		}

		//encontrar goto
		pbar->isgoto_ui = 0;

		index_goto = 0;
		while (index_goto < total_gotos) {
			pgoto = linklist_getindex(x->l_gotos, index_goto);
			if (pgoto->n_bar == i) {
				pbar->isgoto_ui = 1;
				pbar->pgoto_ui = pgoto;
			}
			index_goto++;
		}

		//actualizar posiciones
		pbar->total_notas = total_notas = (long)linklist_getsize(pbar->notas);

		for (int j = 0; j < total_notas; j++) {

			pbar->hdl_nota[j] = linklist_getindex(pbar->notas, j);

			if (cifra_bar > 0) {
				pbar->hdl_nota[j]->pos_ui.x = pbar->hdl_nota[j]->b_inicio / cifra_bar;
				pbar->hdl_nota[j]->pos_ui.y = (labs(pbar->hdl_nota[j]->canal) % 10) * 0.1 + (labs(pbar->hdl_nota[j]->canal) % 9) * 0.01;
			}
			else {
				pbar->hdl_nota[j]->pos_ui.x = pbar->hdl_nota[j]->pos_ui.y = -1;
			}
		}
	}

	jbox_invalidate_layer((t_object *)x, NULL, ps_bars);
	jbox_redraw((t_jbox *)x);
}

void fl_batuta_onoff(t_fl_batuta *x, long n)
{
	if (n) {
		if (x->nota_dirty || x->tempo_dirty || x->cifra_dirty || x->dirty_rec) {
			object_warn((t_object *)x, "actualizar primero");
			return;
		}
		if (x->next_bar_dirty) {
			x->n_bar = x->next_bar - 1;
			x->next_bar_dirty = 0;
			x->samps_beat = 0;
		}
		else { x->n_bar = -1; }

		x->samps_bar = 0;
		x->samps_beat = 0;
		x->index_cifra = x->index_goto = x->index_tempo = x->index_nota = 0;
		x->negras = x->cifras[0]->negras;
		x->ms_beat = x->tempos[0]->ms_beat;
		x->dtempo_busy = 0;
		x->onoff = 1;
	}
	else {
		x->onoff = 0;
		if (x->dirty_rec) {
			fl_batuta_actualizar_rec(x);
		}
		reset_cont_goto(x);
	}
}

void reset_cont_goto(t_fl_batuta *x)
{
	for (int i = 0; i < x->total_gotos; i++) {
		x->gotos[i]->cont_rep = 0;
	}
}

void fl_batuta_nextbar(t_fl_batuta *x, long n)
{
	long n_bar = n;

	reset_cont_goto(x);
	x->next_bar = n_bar;
	x->next_bar_dirty = 1;

	if (!x->onoff) {
		x->jn_bar = n_bar;
		jbox_invalidate_layer((t_object *)x, NULL, ps_bars);
		jbox_redraw((t_jbox *)x);
	}
}

void fl_batuta_rec(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	if (!x->onoff) { return; }

	long n_bar = x->n_bar;
	float b_inicio = (float)(x->samps_bar / (float)x->total_beat);
	t_atom *ap = argv;
	long ac = argc;
	long index_elem_rec;
	fl_nota_rec *nota_rec;
	long n_rec = ac + 2;
	long n_chan = (long)atom_getlong(ap);

	if (x->index_elem_rec >= NELEMS_REC) { object_error((t_object *)x, "rec: no hay mas espacio para grabar"); return; }
	if (ac < 2 || !ap) { object_error((t_object *)x, "rec: 2 argumentos minimo: canal, info..."); return; }

	critical_enter(0);
	index_elem_rec = x->index_elem_rec++;
	x->total_rec++;
	critical_exit(0);

	if (!x->dirty_rec) {
		x->dirty_rec = 1;
	}

	nota_rec = x->rec_data + index_elem_rec;
	nota_rec->ap = (t_atom *)sysmem_resizeptr(nota_rec->ap, n_rec * sizeof(t_atom));
	nota_rec->ac = n_rec;

	atom_setlong(nota_rec->ap, n_bar);
	atom_setfloat(nota_rec->ap + 1, b_inicio);
	atom_setlong(nota_rec->ap + 2, n_chan);
	atom_getatom_array(ac - 1, ap + 1, nota_rec->ac - 3, nota_rec->ap + 3);
}

void fl_batuta_actualizar_rec(t_fl_batuta *x)
{
	fl_nota_rec *nota_rec = x->rec_data;

	for (int i = 0; i < x->total_rec; i++) {
		fl_batuta_nueva_nota(x, NULL, (nota_rec + i)->ac, (nota_rec + i)->ap);
		(nota_rec + i)->ap = (t_atom *)sysmem_resizeptr((nota_rec + i)->ap, sizeof(t_atom));
		(nota_rec + i)->ac = 0;
	}

	x->total_rec = 0;
	x->dirty_rec = 0;
}
