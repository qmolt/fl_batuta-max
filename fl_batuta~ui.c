#include "fl_batuta~.h"

void fl_batuta_tick(t_fl_batuta *x)
{
	x->pos_cursor_norm = x->fasor_cursor;
	jbox_invalidate_layer((t_object *)x, NULL, gensym("cursor_layer"));
	jbox_redraw((t_jbox *)x);

	if (sys_getdspstate()) {	// if the dsp is still on, schedule a next fl_batuta_tick() call
		clock_fdelay(x->cursor_clock, 40); //40ms -> 25fps
	}
}

void fl_batuta_show_range(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv) //chan_sel
{ 
	t_atom *av = argv;
	long ac = argc;
	long a, b;
	if (ac < 1) { return; }
	if (atom_gettype(av) == A_SYM) {
		if (atom_getsym(av) == gensym("all")) { x->jn_show_all_notes = 1; }
		else{ object_warn((t_object *)x, "???"); return; }
	}
	else {
		if (ac == 1) {
			if (atom_gettype(av) != A_FLOAT && atom_gettype(av) != A_LONG) { object_warn((t_object *)x, "channel must be a number"); return; }
			a = (long)atom_getlong(av);
			x->jn_chan_min = x->jn_chan_max = a;
		}
		else if (ac == 2) {
			if (atom_gettype(av) != A_FLOAT && atom_gettype(av) != A_LONG && atom_gettype(av + 1) != A_FLOAT && atom_gettype(av + 1) != A_LONG) { 
				object_warn((t_object *)x, "range values must be numbers"); 
				return; 
			}
			a = (long)atom_getlong(av);
			b = (long)atom_getlong(av + 1);
			x->jn_chan_min = MIN(a, b);
			x->jn_chan_max = MAX(a, b);
		}
		x->jn_show_all_notes = 0;
	}
	jbox_invalidate_layer((t_object *)x, NULL, gensym("bars_layer"));
	jbox_invalidate_layer((t_object *)x, NULL, gensym("notes_layer"));
	jbox_redraw((t_jbox *)x);
}

void fl_batuta_sel_note(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)  //note_sel
{
	t_atom *av = argv;
	long ac = argc;
	t_max_err err = MAX_ERR_NONE;
	long chan;
	long index;
	long bar = x->jn_bar;

	if (ac != 2) { return; }
	if (atom_gettype(av) != A_LONG && atom_gettype(av) != A_FLOAT) { object_warn((t_object *)x, "channel must be a number"); return; }
	if (atom_gettype(av + 1) != A_LONG && atom_gettype(av + 1) != A_FLOAT) { object_warn((t_object *)x, "index must be a number"); return; }

	chan = (long)atom_getlong(av);
	index = (long)atom_getlong(av + 1);

	if (index < 0) { object_warn((t_object *)x, "index must be 0 or positive"); return; }
	
	err = fl_batuta_is_note_index(x, &x->j_selnota, bar, chan, index);
	if(err){ object_warn((t_object *)x, "note couldn't be selected"); return; }

	jbox_invalidate_layer((t_object *)x, NULL, gensym("bars_layer"));
	jbox_invalidate_layer((t_object *)x, NULL, gensym("notes_layer"));
	jbox_redraw((t_jbox *)x);
}

t_max_err fl_batuta_is_note_index(t_fl_batuta *x, long *note_index, long bar, long chan, long index_in_chan)
{ //analog of do_find_note_index(). searches for index on handler. for use in UI after update_notes()
	long n_bar = bar;
	long n_chan = chan;
	long n_note = index_in_chan;
	long total_notes;
	long total_bars = x->total_bars;
	long index_accum = 0;
	long *index_note = note_index;

	*index_note = -1;
	if (n_note < 0) { return MAX_ERR_GENERIC; }
	if (n_bar < 0 || n_bar >= total_bars) { return MAX_ERR_GENERIC; }
	if (!x->bars) { return MAX_ERR_INVALID_PTR; }
	total_notes = x->bars[n_bar]->total_notas;
	if (!x->bars[n_bar]->hdl_nota) { return MAX_ERR_INVALID_PTR; }
	for (int i = 0; i < total_notes; i++) {
		if (x->bars[bar]->hdl_nota[i]->canal == n_chan) {
			if (index_accum++ == n_note) {
				*index_note = i;
			}
		}
	}
	return MAX_ERR_NONE;
}

void fl_batuta_paint(t_fl_batuta *x, t_object *patcherview)
{
	t_object *textfield;

	t_rect rect;
	t_jrgba c;
	t_jgraphics *g = (t_jgraphics *)patcherview_get_jgraphics(patcherview);
	jbox_get_rect_for_view((t_object *)x, patcherview, &rect);

	//achicar margen textfield
	textfield = jbox_get_textfield((t_object *)x);
	if (textfield) {
		textfield_set_textmargins(textfield, 3, 3, 3, rect.height - BOX_TEXTF_H);	// margin on each side
	}

	// draw background
	object_attr_getjrgba((t_object *)x, _sym_bgcolor, &c);
	jgraphics_set_source_jrgba(g, &c);
	jgraphics_rectangle_fill_fast(g, 0, 0, rect.width, rect.height);

	//bars
	fl_batuta_paint_bars(x, patcherview, &rect);
	
	//notes
	fl_batuta_paint_notes(x, patcherview, &rect);

	//cursor
	fl_batuta_paint_cursor(x, patcherview, &rect);

	//textfield
	fl_batuta_paint_textfield(x, patcherview, &rect);

	//info
	fl_batuta_paint_info(x, patcherview, &rect);
}

void fl_batuta_paint_bars(t_fl_batuta *x, t_object *view, t_rect *rect)
{
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("bars_layer"), rect->width, rect->height - BOX_FIJO_H);

	t_jrgba c;

	long divs = 1;
	double cifra_ant = -1., cifra_act = -1., cifra_sig = -1.;

	long jn_bar = x->jn_bar;
	long jn_bar_ant = jn_bar - 1;
	long jn_bar_sig = jn_bar + 1;
	long jn_total_bars = x->jn_total_bars;
	fl_bar *pbar_ant = NULL;
	fl_bar *pbar_act = NULL;
	fl_bar *pbar_sig = NULL;
	char bar_ant_ini = 0;
	char bar_act_ini = 0;
	char bar_sig_ini = 0;
	//check if previous, present, and next bar are valid
	if (x->bars) {
		bar_ant_ini = (jn_bar_ant >= 0 && jn_bar_ant < jn_total_bars) ? 1 : 0;
		bar_act_ini = (jn_bar >= 0 && jn_bar < jn_total_bars) ? 1 : 0;
		bar_sig_ini = (jn_bar_sig >= 0 && jn_bar_sig < jn_total_bars) ? 1 : 0;
		pbar_ant = (bar_ant_ini) ? x->bars[jn_bar_ant] : &x->error_bar;
		pbar_act = (bar_act_ini) ? x->bars[jn_bar] : &x->error_bar;
		pbar_sig = (bar_sig_ini) ? x->bars[jn_bar_sig] : &x->error_bar;
	}
	//in the very extreme case that x->bars couldn't alloc mem in update_bars
	if (!pbar_ant) { pbar_ant = &x->error_bar; }
	if (!pbar_act) { pbar_act = &x->error_bar; }
	if (!pbar_sig) { pbar_sig = &x->error_bar; }
	//if bar is valid and lists non empty, get tsign, tempo and goto
	cifra_ant = pbar_ant->psignat_ui->beats;
	cifra_act = pbar_act->psignat_ui->beats;
	cifra_sig = pbar_sig->psignat_ui->beats;

	//draw layer
	if (g) {
		//rectangles
		double bar_w = (rect->width / 3.) - 2.;
		double bar_h = rect->height - (BOX_FIJO_H + 3.);
		jgraphics_set_source_rgba(g, 0.1, 0.1, 0.1, 0.5);
		if (bar_ant_ini) {
			jgraphics_rectangle_fill_fast(g, 3., 1.5, bar_w, bar_h);
		}
		if (bar_act_ini) {
			jgraphics_rectangle_fill_fast(g, bar_w + 3., 1.5, bar_w, bar_h);
		}
		if (bar_sig_ini) {
			jgraphics_rectangle_fill_fast(g, 2. * bar_w + 3., 1.5, bar_w, bar_h);
		}

		//lines
		object_attr_getjrgba((t_object *)x, gensym("elementcolor"), &c);
		jgraphics_set_source_jrgba(g, &c);

		jgraphics_set_line_width(g, 1.5);
		jgraphics_move_to(g, 3., 1.5);
		jgraphics_line_to(g, 3., bar_h + 1.5);
		jgraphics_stroke(g);

		jgraphics_set_line_width(g, 1.5);
		jgraphics_move_to(g, bar_w + 3., 1.5);
		jgraphics_line_to(g, bar_w + 3., bar_h + 1.5);
		jgraphics_stroke(g);

		jgraphics_set_line_width(g, 1.5);
		jgraphics_move_to(g, 2. * bar_w + 3., 1.5);
		jgraphics_line_to(g, 2. * bar_w + 3., bar_h + 1.5);
		jgraphics_stroke(g);

		jgraphics_set_line_width(g, 1.5);
		jgraphics_move_to(g, 3. * bar_w + 3., 1.5);
		jgraphics_line_to(g, 3. * bar_w + 3., bar_h + 1.5);
		jgraphics_stroke(g);

		//time signature lines
		if (cifra_ant > 0.) {
			object_attr_getjrgba((t_object *)x, gensym("elementcolor"), &c);
			jgraphics_set_source_jrgba(g, &c);
			divs = (long)cifra_ant;
			for (int i = 1; i <= divs; i++) {
				jgraphics_move_to(g, 3. + i * bar_w / cifra_ant, bar_h + 1.5);
				jgraphics_line_to(g, 3. + i * bar_w / cifra_ant, bar_h - 10.);
				jgraphics_set_line_width(g, 1);
				jgraphics_stroke(g);
			}
		}
		if (cifra_act > 0.) {
			object_attr_getjrgba((t_object *)x, gensym("elementcolor"), &c);
			jgraphics_set_source_jrgba(g, &c);
			divs = (long)cifra_act;
			for (int i = 1; i <= divs; i++) {
				jgraphics_move_to(g, 3. + bar_w * (i / cifra_act + 1.), bar_h + 1.5);
				jgraphics_line_to(g, 3. + bar_w * (i / cifra_act + 1.), bar_h - 10.);
				jgraphics_set_line_width(g, 1);
				jgraphics_stroke(g);
			}
		}
		if (cifra_sig > 0.) {
			object_attr_getjrgba((t_object *)x, gensym("elementcolor"), &c);
			jgraphics_set_source_jrgba(g, &c);
			divs = (long)cifra_sig;
			for (int i = 1; i <= divs; i++) {
				jgraphics_move_to(g, 3. + bar_w * (i / cifra_sig + 2.), bar_h + 1.5);
				jgraphics_line_to(g, 3. + bar_w * (i / cifra_sig + 2.), bar_h - 10.);
				jgraphics_set_line_width(g, 1);
				jgraphics_stroke(g);
			}
		}
		jbox_end_layer((t_object *)x, view, gensym("bars_layer"));
	}
	jbox_paint_layer((t_object *)x, view, gensym("bars_layer"), 0., BOX_TEXTF_H);	// position of the layer
}

void fl_batuta_paint_notes(t_fl_batuta *x, t_object *view, t_rect *rect)
{
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("notes_layer"), rect->width, rect->height - BOX_FIJO_H);

	t_jrgba color_nota;
	t_jrgb color_sa;
	double hue;
	double posx, posy, notew, noteh;
	double bar_w, bar_h;
	long total_notas, chan;

	char isplaying = x->isplaying;
	long chan_min = x->jn_chan_min;
	long chan_max = x->jn_chan_max;
	char all_chan = x->jn_show_all_notes;

	long jn_bar = x->jn_bar;
	long jn_bar_ant = jn_bar - 1;
	long jn_bar_sig = jn_bar + 1;
	long jn_total_bars = x->jn_total_bars;
	fl_bar *pbar_ant = NULL;
	fl_bar *pbar_act = NULL;
	fl_bar *pbar_sig = NULL;
	char bar_ant_ini = 0;
	char bar_act_ini = 0;
	char bar_sig_ini = 0;
	//check if previous, present, and next bar are valid
	if (x->bars) {
		bar_ant_ini = (jn_bar_ant >= 0 && jn_bar_ant < jn_total_bars) ? 1 : 0;
		bar_act_ini = (jn_bar >= 0 && jn_bar < jn_total_bars) ? 1 : 0;
		bar_sig_ini = (jn_bar_sig >= 0 && jn_bar_sig < jn_total_bars) ? 1 : 0;
		pbar_ant = (bar_ant_ini) ? x->bars[jn_bar_ant] : &x->error_bar;
		pbar_act = (bar_act_ini) ? x->bars[jn_bar] : &x->error_bar;
		pbar_sig = (bar_sig_ini) ? x->bars[jn_bar_sig] : &x->error_bar;
	}
	//in the very extreme case that x->bars couldn't alloc mem in update_bars
	if (!pbar_ant) { pbar_ant = &x->error_bar; }
	if (!pbar_act) { pbar_act = &x->error_bar; }
	if (!pbar_sig) { pbar_sig = &x->error_bar; }

	//draw layer
	if (g) {
		//rectangles
		bar_w = (rect->width / 3.) - 2.;
		bar_h = rect->height - (BOX_FIJO_H + 3.);
		notew = NOTE_W;
		noteh = bar_h / 10.;
		jgraphics_set_source_rgba(g, 0.1, 0.1, 0.1, 0.5);
		if (bar_ant_ini) {
			total_notas = pbar_ant->total_notas;
			for (int i = 0; i < total_notas; i++) {
				posx = pbar_ant->hdl_nota[i]->pos_ui.x * bar_w + 3.;
				posy = pbar_ant->hdl_nota[i]->pos_ui.y * bar_h + 1.5;
				chan = pbar_ant->hdl_nota[i]->canal;

				if (pbar_ant->hdl_nota[i]->pos_ui.x < 1.) {
					hue = ((chan * COLOR_MULTIPLIER) % 360) / 360.0;
					color_sa = hsltorgb(hue, 0.85, 0.3);
					color_nota.red = color_sa.red;
					color_nota.green = color_sa.green;
					color_nota.blue = color_sa.blue;
					color_nota.alpha = ALPHA_NOTE_ON;

					if (!all_chan && !isplaying) { if (chan < chan_min || chan > chan_max) { color_nota.alpha = ALPHA_NOTE_OFF; } }

					jgraphics_set_source_jrgba(g, &color_nota);	// set the color
					jgraphics_move_to(g, posx, posy);
					jgraphics_rectangle_fill_fast(g, posx, posy, notew, noteh);
				}
				else {
					jgraphics_set_source_rgb(g, 1., 0., 0.);
					jgraphics_line_draw_fast(g, 3., bar_h + 1.5, bar_w + 3., bar_h + 1.5, 1.);
				}
			}
		}
		if (bar_act_ini) {
			total_notas = pbar_act->total_notas;
			for (int i = 0; i < total_notas; i++) {
				posx = pbar_act->hdl_nota[i]->pos_ui.x * bar_w + 3. + bar_w;
				posy = pbar_act->hdl_nota[i]->pos_ui.y * bar_h + 1.5;
				chan = pbar_act->hdl_nota[i]->canal;

				if (pbar_act->hdl_nota[i]->pos_ui.x < 1.) {
					hue = ((chan * COLOR_MULTIPLIER) % 360) / 360.0;
					color_sa = hsltorgb(hue, 0.85, 0.3);
					color_nota.red = color_sa.red;
					color_nota.green = color_sa.green;
					color_nota.blue = color_sa.blue;
					color_nota.alpha = ALPHA_NOTE_ON;

					if (x->j_selnota == i) { color_nota.red = 1.; color_nota.green = 1.; color_nota.blue = 1.; }

					if (!all_chan && !isplaying) { if (chan < chan_min || chan > chan_max) { color_nota.alpha = ALPHA_NOTE_OFF; } }

					jgraphics_set_source_jrgba(g, &color_nota);	// set the color
					jgraphics_move_to(g, posx, posy);
					jgraphics_rectangle_fill_fast(g, posx, posy, notew, noteh);
				}
				else{
					jgraphics_set_source_rgb(g, 1., 0., 0.);
					jgraphics_line_draw_fast(g, bar_w + 3., bar_h + 1.5, 2 * bar_w + 3., bar_h + 1.5, 1.);
				}
			}
		}
		if (bar_sig_ini) {
			total_notas = pbar_sig->total_notas;
			for (int i = 0; i < total_notas; i++) {
				posx = pbar_sig->hdl_nota[i]->pos_ui.x * bar_w + 3. + 2 * bar_w;
				posy = pbar_sig->hdl_nota[i]->pos_ui.y * bar_h + 1.5;
				chan = pbar_sig->hdl_nota[i]->canal;

				if (pbar_sig->hdl_nota[i]->pos_ui.x < 1.) {
					hue = ((chan * COLOR_MULTIPLIER) % 360) / 360.0;
					color_sa = hsltorgb(hue, 0.85, 0.3);
					color_nota.red = color_sa.red;
					color_nota.green = color_sa.green;
					color_nota.blue = color_sa.blue;
					color_nota.alpha = ALPHA_NOTE_ON;

					if (!all_chan && !isplaying) { if (chan < chan_min || chan > chan_max) { color_nota.alpha = ALPHA_NOTE_OFF; } }

					jgraphics_set_source_jrgba(g, &color_nota);	// set the color
					jgraphics_move_to(g, posx, posy);
					jgraphics_rectangle_fill_fast(g, posx, posy, notew, noteh);
				}
				else {
					jgraphics_set_source_rgb(g, 1., 0., 0.);
					jgraphics_line_draw_fast(g, 2. * bar_w + 3., bar_h + 1.5, 3. * bar_w + 3., bar_h + 1.5, 1.);
				}
			}
		}
		jbox_end_layer((t_object *)x, view, gensym("notes_layer"));
	}
	jbox_paint_layer((t_object *)x, view, gensym("notes_layer"), 0., BOX_TEXTF_H);	// position of the layer
}

void fl_batuta_paint_info(t_fl_batuta *x, t_object *view, t_rect *rect)
{
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("info_layer"), rect->width, BOX_INFO_H);

	char text[124];
	
	fl_note *pnote;
	t_max_err err = MAX_ERR_NONE;

	double bar_w = (rect->width / 3.) - 2.;
	
	double ts_ant, ts_act, ts_sig;
	long ts_bar_ant, ts_bar_act, ts_bar_sig;
	short tmp_type_act;
	long tmp_bar_ant, tmp_bar_act, tmp_bar_sig;
	float tmp_curva_act;
	float tmp_delay_act;
	float tmp_duracion_act;
	float tmp_tempo_ant, tmp_tempo_act, tmp_tempo_sig;
	short gt_is_goto_ant, gt_is_goto_act, gt_is_goto_sig;
	long gt_repet_ant, gt_repet_act, gt_repet_sig;
	long gt_tobar_ant, gt_tobar_act, gt_tobar_sig;

	long j_selnota = x->j_selnota;
	long jn_bar = x->jn_bar;
	long jn_bar_ant = jn_bar - 1;
	long jn_bar_sig = jn_bar + 1;
	fl_bar *pbar_ant = NULL;
	fl_bar *pbar_act = NULL;
	fl_bar *pbar_sig = NULL;
	
	long jn_total_tsigns = x->jn_total_tsigns;
	long jn_total_gotos = x->jn_total_gotos;
	long jn_total_tempos = x->jn_total_tempos;
	long jn_total_bars = x->jn_total_bars;
	//check if previous, present, and next bar are valid
	char bar_ant_ini = 0;
	char bar_act_ini = 0;
	char bar_sig_ini = 0;
	if (x->bars) {
		bar_ant_ini = (jn_bar_ant >= 0 && jn_bar_ant < jn_total_bars);
		bar_act_ini = (jn_bar >= 0 && jn_bar < jn_total_bars);
		bar_sig_ini = (jn_bar_sig >= 0 && jn_bar_sig < jn_total_bars);
		pbar_ant = (bar_ant_ini) ? x->bars[jn_bar_ant] : &x->error_bar;
		pbar_act = (bar_act_ini) ? x->bars[jn_bar] : &x->error_bar;
		pbar_sig = (bar_sig_ini) ? x->bars[jn_bar_sig] : &x->error_bar;
	}
	if (!pbar_ant) { pbar_ant = &x->error_bar; }
	if (!pbar_act) { pbar_act = &x->error_bar; }
	if (!pbar_sig) { pbar_sig = &x->error_bar; }

	//get tsign, tempo and goto
	ts_ant = pbar_ant->psignat_ui->beats;
	ts_bar_ant = pbar_ant->psignat_ui->n_bar;
	tmp_bar_ant = pbar_ant->ptempo_ui->n_bar;
	tmp_tempo_ant = pbar_ant->ptempo_ui->ms_beat;
	gt_is_goto_ant = pbar_ant->isgoto_ui;
	if (gt_is_goto_ant > 0) {
		gt_repet_ant = pbar_ant->pgoto_ui->total_rep - pbar_ant->pgoto_ui->cont_rep;
		gt_tobar_ant = pbar_ant->pgoto_ui->to_bar;
	}

	ts_act = pbar_act->psignat_ui->beats;
	ts_bar_act = pbar_act->psignat_ui->n_bar;
	tmp_bar_act = pbar_act->ptempo_ui->n_bar;
	tmp_type_act = pbar_act->ptempo_ui->type;
	tmp_curva_act = pbar_act->ptempo_ui->curva;
	tmp_tempo_act = pbar_act->ptempo_ui->ms_beat;
	tmp_delay_act = pbar_act->ptempo_ui->ms_inicio;
	tmp_duracion_act = pbar_act->ptempo_ui->ms_durvar;
	gt_is_goto_act = pbar_act->isgoto_ui;
	if (gt_is_goto_act > 0) {
		gt_repet_act = pbar_act->pgoto_ui->total_rep - pbar_act->pgoto_ui->cont_rep;
		gt_tobar_act = pbar_act->pgoto_ui->to_bar;
	}

	ts_sig = pbar_sig->psignat_ui->beats;
	ts_bar_sig = pbar_sig->psignat_ui->n_bar;
	tmp_bar_sig = pbar_sig->ptempo_ui->n_bar;
	tmp_tempo_sig = pbar_sig->ptempo_ui->ms_beat;
	gt_is_goto_sig = pbar_sig->isgoto_ui;
	if (gt_is_goto_sig > 0) {
		gt_repet_sig = pbar_sig->pgoto_ui->total_rep - pbar_sig->pgoto_ui->cont_rep;
		gt_tobar_sig = pbar_sig->pgoto_ui->to_bar;
	}

	//draw layer
	if (g) {
		//text
		jgraphics_set_line_width(g, 2.0);
		jgraphics_select_font_face(g, "Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_BOLD);
		jgraphics_set_font_size(g, 9.);
		//time signature text
		jgraphics_set_source_rgb(g, 1., 1., 1.);
		if (ts_ant > 0) {
			jgraphics_move_to(g, 70, 0.2 * BOX_INFO_H);
			if (ts_bar_ant == jn_bar_ant) { sprintf(text, "♦ %8.5f", ts_ant); }
			else { sprintf(text, "%8.5f", ts_ant); }
			jgraphics_show_text(g, text);
		}
		if (ts_act > 0) {
			jgraphics_move_to(g, bar_w, 0.2 * BOX_INFO_H);
			if (ts_bar_act == jn_bar) { sprintf(text, "♦ %8.5f", ts_act); }
			else { sprintf(text, "%8.5f", ts_act); }
			jgraphics_show_text(g, text);
		}
		if (ts_sig > 0) {
			jgraphics_move_to(g, 2. * bar_w, 0.2 * BOX_INFO_H);
			if (ts_bar_sig == jn_bar_sig) { sprintf(text, "♦ %8.5f", ts_sig); }
			else { sprintf(text, "%8.5f", ts_sig); }
			jgraphics_show_text(g, text);
		}
		jgraphics_set_source_rgb(g, .8, .9, .9);
		if (!jn_total_tsigns) { jgraphics_set_source_rgb(g, 1., 0., 0.); }
		jgraphics_move_to(g, 3., 0.2 * BOX_INFO_H);
		jgraphics_show_text(g, "time sign.");

		//texto tempo
		jgraphics_set_source_rgb(g, 1., 1., 1.);
		if (tmp_bar_ant >= 0) {
			jgraphics_move_to(g, 70., 0.4 * BOX_INFO_H);
			if (tmp_bar_ant == jn_bar_ant) { sprintf(text, "♦ %8.2f", tmp_tempo_ant); }
			else { sprintf(text, "%8.2f", tmp_tempo_ant); }
			jgraphics_show_text(g, text);
		}
		if (tmp_bar_act >= 0) {
			jgraphics_move_to(g, bar_w, 0.4 * BOX_INFO_H);
			if (tmp_bar_act == jn_bar) {
				if (tmp_type_act == 0) { sprintf(text, "♦ %8.2f [→%8.1f]", tmp_tempo_act, tmp_delay_act); }
				else if (tmp_type_act == 1) { sprintf(text, "♦ %8.2f [→%8.1f, ∆%8.2f]", tmp_tempo_act, tmp_delay_act, tmp_duracion_act); }
				else if (tmp_type_act == 2) { sprintf(text, "♦ %8.2f [→%8.1f, ∆%8.2f, c%3.2f]", tmp_tempo_act, tmp_delay_act, tmp_duracion_act, tmp_curva_act); }
			}
			else { sprintf(text, "%8.2f", tmp_tempo_act); }
			jgraphics_show_text(g, text);
		}
		if (tmp_bar_sig >= 0) {
			jgraphics_move_to(g, 2. * bar_w, 0.4 * BOX_INFO_H);
			if (tmp_bar_sig == jn_bar_sig) { sprintf(text, "♦ %8.2f", tmp_tempo_sig); }
			else { sprintf(text, "%8.2f", tmp_tempo_sig); }
			jgraphics_show_text(g, text);
		}
		jgraphics_set_source_rgb(g, .8, .9, .9);
		if (!jn_total_tempos) { jgraphics_set_source_rgb(g, 1., 0., 0.); }
		jgraphics_move_to(g, 3, 0.4 * BOX_INFO_H);
		jgraphics_show_text(g, "tempo");

		//texto goto
		jgraphics_set_source_rgb(g, 1., 1., 1.);
		if (gt_is_goto_ant > 0) {
			jgraphics_move_to(g, 70., 0.6 * BOX_INFO_H);
			sprintf(text, "♦ %d [X%d]", gt_tobar_ant, gt_repet_ant);
			jgraphics_show_text(g, text);
		}
		if (gt_is_goto_act > 0) {
			jgraphics_move_to(g, bar_w, 0.6 * BOX_INFO_H);
			sprintf(text, "♦ %d [X%d]", gt_tobar_act, gt_repet_act);
			jgraphics_show_text(g, text);
		}
		if (gt_is_goto_sig > 0) {
			jgraphics_move_to(g, 2 * bar_w, 0.6 * BOX_INFO_H);
			sprintf(text, "♦ %d [X%d]", gt_tobar_sig, gt_repet_sig);
			jgraphics_show_text(g, text);
		}
		jgraphics_set_source_rgb(g, .8, .9, .9);
		jgraphics_move_to(g, 3., 0.6 * BOX_INFO_H);
		jgraphics_show_text(g, "go to");

		//texto compas
		jgraphics_set_source_rgb(g, 0.8, 0.8, 0.2);
		jgraphics_move_to(g, bar_w - 27, 0.8 * BOX_INFO_H);
		jgraphics_show_text(g, "bar");
		jgraphics_move_to(g, bar_w, 0.8 * BOX_INFO_H);
		sprintf(text, "%d", jn_bar);
		jgraphics_show_text(g, text);

		//status
		jgraphics_set_source_rgb(g, .8, .9, .9);
		jgraphics_move_to(g, 3., 0.8 * BOX_INFO_H);
		jgraphics_show_text(g, "status");
		jgraphics_set_source_rgb(g, 1., 1., 1.);
		jgraphics_move_to(g, 35., 0.82 * BOX_INFO_H);
		if (x->isplaying) {
			jgraphics_set_font_size(g, 11.);
			jgraphics_show_text(g, "►");
		}
		else {
			jgraphics_set_font_size(g, 15.);
			jgraphics_show_text(g, "■");
		}
		if (x->dirty_rec) {
			jgraphics_set_source_rgb(g, 1., 0., 0.);
			jgraphics_move_to(g, 38., 0.82 * BOX_INFO_H);
			jgraphics_set_font_size(g, 17.);
			jgraphics_show_text(g, "●");
		}

		//selected note info
		if (x->j_selnota >= 0) {
			jgraphics_set_font_size(g, 8.5);
			jgraphics_set_source_rgb(g, .9, .8, .9);
			jgraphics_move_to(g, 2. * bar_w + 3., 0.8 * BOX_INFO_H);
			pnote = pbar_act->hdl_nota[j_selnota];
			err = atom_gettext(pnote->cnota, pnote->pnota, &x->text_size, &x->text_info, 0);
			if (!err) {
				sprintf(text, "beat:%7.5f, info: %s", pnote->b_inicio, x->text_info);
				jgraphics_show_text(g, text);
			}
		}

		jbox_end_layer((t_object *)x, view, gensym("info_layer"));
	}
	jbox_paint_layer((t_object *)x, view, gensym("info_layer"), 0., rect->height-BOX_INFO_H);	// position of the layer
}
void fl_batuta_paint_cursor(t_fl_batuta *x, t_object *view, t_rect *rect)
{
	double pos_cursor;
	t_jrgba c;
	double bar_w = (rect->width / 3.) - 2.;
	double bar_h = rect->height - (BOX_FIJO_H + 3.);
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("cursor_layer"), bar_w, rect->height - BOX_FIJO_H);

	if (g) {
		//cursor
		object_attr_getjrgba((t_object *)x, gensym("cursorcolor"), &c);
		jgraphics_set_source_jrgba(g, &c);

		pos_cursor = x->pos_cursor_norm * bar_w;
		jgraphics_set_line_width(g, 2.);
		jgraphics_move_to(g, pos_cursor, 1.5);
		jgraphics_line_to(g, pos_cursor, bar_h + 1.5);
		jgraphics_stroke(g);
		jbox_end_layer((t_object *)x, view, gensym("cursor_layer"));
	}
	jbox_paint_layer((t_object *)x, view, gensym("cursor_layer"), bar_w + 3., BOX_TEXTF_H);	// position of the layer
}
void fl_batuta_paint_textfield(t_fl_batuta *x, t_object *view, t_rect *rect)
{
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("textin_layer"), rect->width, BOX_TEXTF_H);
	t_jrgba c;

	if (g) {
		// set up matrix
		//jbox_get_rect_for_view((t_object *)x, view, &rect);

		// soft gray background
		object_attr_getjrgba((t_object *)x, gensym("elementcolor"), &c);
		jgraphics_rectangle(g, 3., 3., rect->width - 6, BOX_TEXTF_H - 4.5);
		jgraphics_set_source_jrgba(g, &c);
		jgraphics_fill(g);

		// set line width for the corners
		jgraphics_set_line_width(g, 2.);
		jgraphics_set_source_rgba(g, x->j_textbg.red * 0.7, x->j_textbg.green * 0.7, x->j_textbg.blue * 0.7, x->j_textbg.alpha * 0.7);

		// draw top left corner
		jgraphics_move_to(g, 1., rect->height - 8.);
		jgraphics_line_to(g, 1., rect->height - 1.);
		jgraphics_line_to(g, 8., rect->height - 1.);
		jgraphics_stroke(g);

		// draw bottom right corner
		jgraphics_move_to(g, rect->width - 8., 1.);
		jgraphics_line_to(g, rect->width - 1., 1.);
		jgraphics_line_to(g, rect->width - 1., 8.);
		jgraphics_stroke(g);
		jbox_end_layer((t_object *)x, view, gensym("textin_layer"));
	}
	jbox_paint_layer((t_object *)x, view, gensym("textin_layer"), 0., 0.);	// position of the layer
}

/*
void fl_batuta_mousemove(t_fl_batuta *x, t_object *patcherview, t_pt pt, long modifiers){}

void fl_batuta_mouseleave(t_fl_batuta *x, t_object *patcherview, t_pt pt, long modifiers)
{
	if (x->isplaying) { return; }

	jbox_invalidate_layer((t_object *)x, NULL, gensym("bars_layer"));
	jbox_invalidate_layer((t_object *)x, NULL, gensym("notes_layer"));
	jbox_redraw((t_jbox *)x);
}

void fl_batuta_mousedown(t_fl_batuta *x, t_object *patcherview, t_pt pt, long modifiers)
{
	if (x->isplaying) { return; }

	jbox_invalidate_layer((t_object *)x, NULL, gensym("bars_layer"));
	jbox_invalidate_layer((t_object *)x, NULL, gensym("notes_layer"));
	jbox_redraw((t_jbox *)x);
}

t_max_err fl_batuta_notify(t_fl_batuta *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	if (s == gensym("attr_modified")) {
		t_symbol *name = (t_symbol *)object_method((t_object *)data, gensym("getname"));

		if (name == gensym("rectcolor"))
			jbox_invalidate_layer((t_object *)x, NULL, ps_puntos);
	}
	return jbox_notify((t_jbox *)x, s, msg, sender, data);
}
*/