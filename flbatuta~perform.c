#include "flbatuta~.h"

t_max_err fl_batuta_update_notes_onebar(t_fl_batuta *x, long n)
{
	fl_bar *pbar;
	long total_notes;

	pbar = linklist_getindex(x->l_bars, n);
	if (!pbar) { return MAX_ERR_GENERIC; }
	x->bars[n] = pbar;
	total_notes = (long)linklist_getsize(pbar->notas);

	if (total_notes == 0) { return MAX_ERR_NONE; }

	linklist_sort(pbar->notas, note_prevstart);

	if (!pbar->hdl_nota) { pbar->hdl_nota = (fl_note **)sysmem_newptr(total_notes * sizeof(fl_note *)); }
	else { pbar->hdl_nota = (fl_note **)sysmem_resizeptr(pbar->hdl_nota, total_notes * sizeof(fl_note *)); }
	if (!pbar->hdl_nota) { pbar->total_notas = -1; return MAX_ERR_OUT_OF_MEM; }

	for (int j = 0; j < total_notes; j++) { pbar->hdl_nota[j] = linklist_getindex(pbar->notas, j); }
	pbar->total_notas = total_notes;

	return MAX_ERR_NONE;
}

t_max_err fl_batuta_update_notes(t_fl_batuta *x)
{
	fl_bar *pbar;
	long total_bars;
	long total_notes;

	total_bars = (long)linklist_getsize(x->l_bars);
	if (total_bars == 0) {
		if (!x->bars) { x->bars = (fl_bar **)sysmem_newptr(sizeof(fl_bar *)); }
		else { x->bars = (fl_bar **)sysmem_resizeptr(x->bars, sizeof(fl_bar *)); }
		if (!x->bars) { x->total_bars = -1; return MAX_ERR_OUT_OF_MEM; }
		x->bars[0] = &x->error_bar;
	}
	else {
		x->bars = (fl_bar **)sysmem_resizeptr(x->bars, total_bars * sizeof(fl_bar *));
		if (!x->bars) { x->total_bars = -1; return MAX_ERR_OUT_OF_MEM; }
	}

	for (int i = 0; i < total_bars; i++) {
		pbar = linklist_getindex(x->l_bars, i);
		if (!pbar) { return MAX_ERR_GENERIC; }
		x->bars[i] = pbar;
		total_notes = (long)linklist_getsize(pbar->notas);

		if (total_notes == 0) { continue; }

		linklist_sort(pbar->notas, note_prevstart);

		if (!pbar->hdl_nota) { pbar->hdl_nota = (fl_note **)sysmem_newptr(total_notes * sizeof(fl_note *)); }
		else { pbar->hdl_nota = (fl_note **)sysmem_resizeptr(pbar->hdl_nota, total_notes * sizeof(fl_note *)); }
		if (!pbar->hdl_nota) { pbar->total_notas = -1; return MAX_ERR_OUT_OF_MEM; }

		for (int j = 0; j < total_notes; j++) { pbar->hdl_nota[j] = linklist_getindex(pbar->notas, j); }
		pbar->total_notas = total_notes;
	}
	x->total_bars = total_bars;
	return MAX_ERR_NONE;
}

t_max_err fl_batuta_update_tempos(t_fl_batuta *x)
{
	long total_tempos = (long)linklist_getsize(x->l_tempos);
	if (!x->tempos) { x->tempos = (fl_tempo **)sysmem_newptr(total_tempos * sizeof(fl_tempo *)); }
	else{ x->tempos = (fl_tempo **)sysmem_resizeptr(x->tempos, total_tempos * sizeof(fl_tempo *)); }
	if (!x->tempos) { return MAX_ERR_OUT_OF_MEM; }
	for (int i = 0; i < total_tempos; i++) {
		x->tempos[i] = linklist_getindex(x->l_tempos, i);
		if (!x->tempos[i]) { return MAX_ERR_GENERIC; }
	}
	x->total_tempos = total_tempos;
	return MAX_ERR_NONE;
}

t_max_err fl_batuta_update_signatures(t_fl_batuta *x)
{
	long total_tsigns = (long)linklist_getsize(x->l_tsigns);
	if (!x->tsigns) { x->tsigns = (fl_tsign **)sysmem_newptr(total_tsigns * sizeof(fl_tsign *)); }
	else { x->tsigns = (fl_tsign **)sysmem_resizeptr(x->tsigns, total_tsigns * sizeof(fl_tsign *)); }
	if (!x->tsigns) { return MAX_ERR_OUT_OF_MEM; }
	for (int i = 0; i < total_tsigns; i++) {
		x->tsigns[i] = linklist_getindex(x->l_tsigns, i);
		if (!x->tsigns[i]) { return MAX_ERR_GENERIC; }
	}
	x->total_tsigns = total_tsigns;
	return MAX_ERR_NONE;
}

t_max_err fl_batuta_update_gotos(t_fl_batuta *x)
{
	long total_gotos = (long)linklist_getsize(x->l_gotos);
	if (!x->gotos) { x->gotos = (fl_goto **)sysmem_newptr(total_gotos * sizeof(fl_goto *)); }
	else { x->gotos = (fl_goto **)sysmem_resizeptr(x->gotos, total_gotos * sizeof(fl_goto *)); }
	if (!x->gotos) { return MAX_ERR_OUT_OF_MEM; }
	for (int i = 0; i < total_gotos; i++) {
		x->gotos[i] = linklist_getindex(x->l_gotos, i);
		if (!x->gotos[i]) { return MAX_ERR_GENERIC; }
		x->gotos[i]->cont_rep = 0;
	}
	x->total_gotos = total_gotos;
	return MAX_ERR_NONE;
}

t_max_err fl_batuta_update_uitempo(t_fl_batuta *x) 
{
	long index_tempo;
	fl_bar *pbar;
	fl_tempo *ptempo;
	long total_tempos = x->total_tempos;
	long total_bars = x->total_bars;

	for (int i = 0; i < total_bars; i++) {
		pbar = x->bars[i];
		if (!pbar) { return MAX_ERR_INVALID_PTR; }
		pbar->ptempo_ui = &x->error_tempo;
		index_tempo = 0;
		while (index_tempo < total_tempos) {//find last tempo for each bar
			ptempo = linklist_getindex(x->l_tempos, index_tempo);
			if (!ptempo) { return MAX_ERR_INVALID_PTR; }
			if (ptempo->n_bar <= i) { pbar->ptempo_ui = ptempo; }
			index_tempo++;
		}
	}
	x->jn_total_tempos = total_tempos;

	jbox_invalidate_layer((t_object *)x, NULL, gensym("info_layer"));
	jbox_redraw((t_jbox *)x);

	return MAX_ERR_NONE;
}
t_max_err fl_batuta_update_uigoto(t_fl_batuta *x){
	long index_goto;
	fl_bar *pbar;
	fl_goto *pgoto;

	long total_bars = x->total_bars;
	long total_gotos = x->total_gotos;

	for (int i = 0; i < total_bars; i++) {
		pbar = x->bars[i];
		if (!pbar) { return MAX_ERR_INVALID_PTR; }
		pbar->pgoto_ui = &x->error_goto;
		pbar->isgoto_ui = 0;
		//find gotos
		index_goto = 0;
		while (index_goto < total_gotos) {
			pgoto = linklist_getindex(x->l_gotos, index_goto);
			if (!pgoto) { return MAX_ERR_INVALID_PTR; }
			if (pgoto->n_bar == i) {
				pbar->pgoto_ui = pgoto;
				pbar->isgoto_ui = 1;
			}
			index_goto++;
		}
	}
	x->jn_total_gotos = total_gotos;

	jbox_invalidate_layer((t_object *)x, NULL, gensym("info_layer"));
	jbox_redraw((t_jbox *)x);

	return MAX_ERR_NONE;
}
t_max_err fl_batuta_update_uibar(t_fl_batuta *x)
{
	long index_cifra;
	fl_tsign *pcifra;
	fl_bar *pbar;
	long total_notas;
	float cifra = 0.f;

	long total_bars = x->total_bars;
	long total_tsigns = x->total_tsigns;

	for (int i = 0; i < total_bars; i++) {
		pbar = x->bars[i];
		if (!pbar) { return MAX_ERR_INVALID_PTR; }
		pbar->psignat_ui = &x->error_tsign;
		index_cifra = 0;
		while (index_cifra < total_tsigns) {//find last signature for each bar
			pcifra = linklist_getindex(x->l_tsigns, index_cifra);
			if (!pcifra) { return MAX_ERR_INVALID_PTR; }
			if (pcifra->n_bar <= i) { pbar->psignat_ui = pcifra; }
			index_cifra++;
		}
		
		total_notas = (long)linklist_getsize(pbar->notas);
		for (int j = 0; j < total_notas; j++) {//position for each note
			pbar->hdl_nota[j] = linklist_getindex(pbar->notas, j);
			if (!pbar->hdl_nota[j]) { return MAX_ERR_INVALID_PTR; }
			cifra = pbar->psignat_ui->beats;
			if (cifra > 0.) { //x,y norm [0,1]
				pbar->hdl_nota[j]->pos_ui.x = pbar->hdl_nota[j]->b_inicio / cifra;
				pbar->hdl_nota[j]->pos_ui.y = (labs(pbar->hdl_nota[j]->canal) % 10) * 0.1 + ((labs(pbar->hdl_nota[j]->canal) / 10) % 9) * 0.01;
			}
			else {
				pbar->hdl_nota[j]->pos_ui.x = pbar->hdl_nota[j]->pos_ui.y = -1.;
			}
		}
	}
	x->jn_total_tsigns = total_tsigns;
	x->jn_total_bars = total_bars;

	jbox_invalidate_layer((t_object *)x, NULL, gensym("bars_layer"));
	jbox_invalidate_layer((t_object *)x, NULL, gensym("notes_layer"));
	jbox_invalidate_layer((t_object *)x, NULL, gensym("info_layer"));
	jbox_redraw((t_jbox *)x);

	return MAX_ERR_NONE;
}

//---------------------------------------------
void fl_batuta_onoff(t_fl_batuta *x, long n)
{
	if (n != n) { return; }

	if (n < 0) { n = 0; }
	if (n) {
		if (x->isediting || x->isloading) {
			object_error((t_object *)x, "play: can't play while editing or loading. Try again");
			return;
		}

		if (x->next_bar_dirty) {
			x->n_bar = x->next_bar;
			x->next_bar_dirty = 0;
			x->samps_beat = 0;
		}
		else { x->n_bar = 0; }

		x->samps_bar = 0;
		x->samps_beat = 0;
		x->index_cifra = x->index_goto = x->index_tempo = x->index_nota = 0;
		x->negras = x->tsigns[0]->beats;
		x->ms_beat = x->tempos[0]->ms_beat;
		x->task_tempo = 0;

		x->isplaying = 1;
		x->task_out_bar = 1;
		x->task_new_idx = 0;//debug
	}
	else {
		x->isplaying = 0;
		if (x->dirty_rec) {
			fl_batuta_update_rec(x);
		}
		reset_cont_goto(x);
	}
	jbox_invalidate_layer((t_object *)x, NULL, gensym("info_layer"));
	jbox_redraw((t_jbox *)x);
}

void reset_cont_goto(t_fl_batuta *x)
{
	for (int i = 0; i < x->total_gotos; i++) {
		x->gotos[i]->cont_rep = 0;
	}
}

void fl_batuta_nextbar(t_fl_batuta *x, long n)
{
	if (n != n) { return; }
	
	if (n < 0) { n = 0; }

	x->next_bar = n;
	x->next_bar_dirty = 1;

	x->j_selnota = -1;
	if (!x->isplaying) {
		x->jn_bar = n;
		jbox_invalidate_layer((t_object *)x, NULL, gensym("bars_layer"));
		jbox_invalidate_layer((t_object *)x, NULL, gensym("notes_layer"));
		jbox_invalidate_layer((t_object *)x, NULL, gensym("info_layer"));
		jbox_redraw((t_jbox *)x);
	}
}

void fl_batuta_rec(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	if (!x->isplaying) { return; }

	long n_bar = x->n_bar;
	float b_inicio = (float)(x->samps_bar / (float)x->total_beat);
	t_atom *ap = argv;
	long ac = argc;
	long index_elem_rec;
	fl_note_rec *nota_rec;

	if (x->index_elem_rec >= NELEMS_REC) { object_error((t_object *)x, "rec: ran out of memory"); return; }
	if (ac < 2 || !ap) { object_error((t_object *)x, "rec: [2 args min] channel number, info..."); return; }

	critical_enter(0);
	index_elem_rec = x->index_elem_rec++;
	x->total_rec++;
	critical_exit(0);

	if (!x->dirty_rec) {
		x->dirty_rec = 1;
	}

	nota_rec = &(x->rec_data[index_elem_rec]);
	if (!nota_rec->ap) { (t_atom *)sysmem_newptr((ac - 1) * sizeof(t_atom)); }
	else { nota_rec->ap = (t_atom *)sysmem_resizeptr(nota_rec->ap, (ac - 1) * sizeof(t_atom)); }
	nota_rec->ac = ac - 1;

	nota_rec->bar = n_bar;
	nota_rec->b_start = b_inicio;
	atom_getatom_array(ac - 1, ap + 1, nota_rec->ac, nota_rec->ap); //info...
}

void fl_batuta_update_rec(t_fl_batuta *x)
{
	fl_note_rec *nota_rec = x->rec_data;
	long chan = 0;

	x->isloading = 1;

	for (int i = 0; i < x->total_rec; i++) {
		chan = (long)atom_getlong(nota_rec[i].ap);
		do_add_note(x, nota_rec[i].bar, nota_rec[i].b_start, chan, nota_rec[i].ac, nota_rec[i].ap, 1);
		if (!nota_rec[i].ap) { nota_rec[i].ap = (t_atom *)sysmem_newptr(sizeof(t_atom)); }
		else { nota_rec[i].ap = (t_atom *)sysmem_resizeptr(nota_rec[i].ap, sizeof(t_atom)); }
		if (!nota_rec[i].ap) { object_error((t_object *)x, "rec: ran out of memory"); return; }
		nota_rec[i].ac = 0;
	}

	fl_batuta_update_notes(x);
	fl_batuta_update_uibar(x);

	x->isloading = 0;
	x->total_rec = 0;
	x->dirty_rec = 0;
}
