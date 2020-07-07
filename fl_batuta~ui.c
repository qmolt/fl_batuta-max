#include "fl_batuta~.h"

void fl_batuta_tick(t_fl_batuta *x)
{
	if (x->pos_cursor_norm != x->fasor_cursor) {
		x->pos_cursor_norm = x->fasor_cursor;
		jbox_invalidate_layer((t_object *)x, NULL, ps_cursor_layer);
		jbox_redraw((t_jbox *)x);
	}

	if (sys_getdspstate()) {	// if the dsp is still on, schedule a next pictmeter_tick() call
		clock_fdelay(x->cursor_clock, 50);
	}
}

void fl_batuta_paint(t_fl_batuta *x, t_object *patcherview)
{
	// paint the box grey
	long ii, jj, jn_bar, jn_total_bars;
	double posx, posy;
	fl_bar *pbar;
	t_object *textfield;
	char *text, *text_info;
	long text_len;

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

	//compases
	fl_batuta_paint_bars(x, patcherview, &rect);

	//cursor
	fl_batuta_paint_cursor(x, patcherview, &rect);

	//textfield
	fl_batuta_paint_textfield(x, patcherview, &rect);

	//over triangle
	if (x->onoff) { return; }

	ii = x->j_overnota;
	jj = x->j_selnota;
	jn_bar = x->jn_bar;
	jn_total_bars = x->jn_total_bars;

	if (ii >= 0) {
		if (jn_bar >= 0 && jn_bar < jn_total_bars) {
			text = (char *)sysmem_newptr(LARGO_MAX_LINEA * sizeof(char));
			text_info = (char *)sysmem_newptr(LARGO_MAX_LINEA * sizeof(char));

			pbar = x->compases[jn_bar];
			posx = (pbar->hdl_nota[ii]->pos_ui.x + 1) * rect.width / 3;
			posy = pbar->hdl_nota[ii]->pos_ui.y * (rect.height - BOX_FIJO_H - LADO_CUADRADO) + BOX_TEXTF_H;

			jgraphics_set_source_rgba(g, 0.7, 0.7, 0.7, 1.);;
			jgraphics_move_to(g, posx, posy);
			jgraphics_line_to(g, posx + LADO_CUADRADO, posy + LADO_CUADRADO * 0.5);
			jgraphics_line_to(g, posx, posy + LADO_CUADRADO);
			jgraphics_line_to(g, posx, posy);
			jgraphics_close_path(g);
			jgraphics_fill(g);

			jgraphics_set_line_width(g, 2.0);
			jgraphics_select_font_face(g, "Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_BOLD);
			jgraphics_set_font_size(g, 8.5);
			jgraphics_set_source_rgb(g, 1., 1., 1.);
			jgraphics_move_to(g, rect.width / 3, rect.height - 0.2 * BOX_INFO_H);
			my_gettext(pbar->hdl_nota[ii]->cnota, pbar->hdl_nota[ii]->pnota, &text_len, &text_info, 0);
			sprintf(text, "inicio: %7.5f; canal e info: %s", pbar->hdl_nota[ii]->b_inicio, text_info);
			jgraphics_show_text(g, text);

			sysmem_freeptr(text);
			sysmem_freeptr(text_info);
		}
	}
	if (jj >= 0) {
		if (jn_bar >= 0 && jn_bar < jn_total_bars) {
			pbar = x->compases[jn_bar];
			posx = (pbar->hdl_nota[jj]->pos_ui.x + 1) * rect.width / 3;
			posy = pbar->hdl_nota[jj]->pos_ui.y * (rect.height - BOX_FIJO_H - LADO_CUADRADO) + BOX_TEXTF_H;

			jgraphics_set_source_rgba(g, 1., 1., 1., 1.);;
			jgraphics_move_to(g, posx, posy);
			jgraphics_line_to(g, posx + LADO_CUADRADO, posy + LADO_CUADRADO * 0.5);
			jgraphics_line_to(g, posx, posy + LADO_CUADRADO);
			jgraphics_line_to(g, posx, posy);
			jgraphics_close_path(g);
			jgraphics_fill(g);
		}
	}
}
void fl_batuta_paint_bars(t_fl_batuta *x, t_object *view, t_rect *rect)
{
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, ps_bars, rect->width, rect->height);

	char text[80];
	t_jrgba c;

	t_jrgba color_nota;
	t_jrgb color_sa;
	double hue;
	double posx, posy;
	long total_notas, canal;

	long jn_bar = x->jn_bar;
	long jn_bar_ant = jn_bar - 1;
	long jn_bar_sig = jn_bar + 1;
	long jn_total_bars = x->jn_total_bars;
	fl_bar *pbar_ant = NULL;
	fl_bar *pbar_act = NULL;
	fl_bar *pbar_sig = NULL;
	fl_bar *pbar = NULL;
	short is_ini;
	short jn_bar_ant_in = (jn_bar_ant >= 0 && jn_bar_ant < jn_total_bars) ? 1 : 0;
	short jn_bar_act_in = (jn_bar >= 0 && jn_bar < jn_total_bars) ? 1 : 0;
	short jn_bar_sig_in = (jn_bar_sig >= 0 && jn_bar_sig < jn_total_bars) ? 1 : 0;

	long divs = 1;
	float cifra_ant = -1, cifra_act = -1, cifra_sig = -1;
	short tmp_type_ant = -1, tmp_type_act = -1, tmp_type_sig = -1;
	float tmp_curva_ant, tmp_curva_act, tmp_curva_sig;
	float tmp_tempo_ant, tmp_tempo_act, tmp_tempo_sig;
	float tmp_delay_ant, tmp_delay_act, tmp_delay_sig;
	float tmp_duracion_ant, tmp_duracion_act, tmp_duracion_sig;
	short gt_is_goto_act = 0, gt_is_goto_sig = 0;
	long gt_repet_sig;
	long gt_tobar_sig;

	is_ini = (x->jn_tmpo_ini && x->jn_cfra_ini && x->jn_nota_ini) ? 1 : 0;

	pbar_ant = (jn_bar_ant < jn_total_bars &&jn_bar_ant >= 0 && is_ini) ? x->compases[jn_bar_ant] : x->pbar_error;
	pbar_act = (jn_bar < jn_total_bars &&jn_bar >= 0 && is_ini) ? x->compases[jn_bar] : x->pbar_error;
	pbar_sig = (jn_bar_ant < jn_total_bars &&jn_bar_sig >= 0 && is_ini) ? x->compases[jn_bar_sig] : x->pbar_error;

	//cifra, tempo y goto
	if (jn_bar_ant_in && is_ini) {
		cifra_ant = pbar_ant->cifra_ui;
		tmp_type_ant = pbar_ant->tempo_ui.type;
		tmp_curva_ant = pbar_ant->tempo_ui.curva;
		tmp_tempo_ant = pbar_ant->tempo_ui.ms_beat;
		tmp_delay_ant = pbar_ant->tempo_ui.ms_inicio;
		tmp_duracion_ant = pbar_ant->tempo_ui.ms_durvar;
	}
	if (jn_bar_act_in && is_ini) {
		cifra_act = pbar_act->cifra_ui;
		tmp_type_act = pbar_act->tempo_ui.type;
		tmp_curva_act = pbar_act->tempo_ui.curva;
		tmp_tempo_act = pbar_act->tempo_ui.ms_beat;
		tmp_delay_act = pbar_act->tempo_ui.ms_inicio;
		tmp_duracion_act = pbar_act->tempo_ui.ms_durvar;
		gt_is_goto_act = pbar_act->isgoto_ui;
	}
	if (jn_bar_sig_in && is_ini) {
		cifra_sig = pbar_sig->cifra_ui;
		tmp_type_sig = pbar_sig->tempo_ui.type;
		tmp_curva_sig = pbar_sig->tempo_ui.curva;
		tmp_tempo_sig = pbar_sig->tempo_ui.ms_beat;
		tmp_delay_sig = pbar_sig->tempo_ui.ms_inicio;
		tmp_duracion_sig = pbar_sig->tempo_ui.ms_durvar;
		gt_is_goto_sig = pbar_sig->isgoto_ui;
		if (gt_is_goto_sig) {
			gt_repet_sig = pbar_sig->pgoto_ui->total_rep - pbar_sig->pgoto_ui->cont_rep;
			gt_tobar_sig = pbar_sig->pgoto_ui->to_bar;
		}
	}

	if (g) {
		//cuadrados
		if (!x->jn_nota_ini || !jn_bar_ant_in) {
			jgraphics_set_source_rgba(g, 0.1, 0.1, 0.1, 0.5);
			jgraphics_rectangle_fill_fast(g, 3., BOX_TEXTF_H + 3., rect->width / 3 - 3, rect->height - BOX_FIJO_H - 3);
		}
		if (!x->jn_nota_ini || !jn_bar_act_in) {
			jgraphics_set_source_rgba(g, 0.1, 0.1, 0.1, 0.5);
			jgraphics_rectangle_fill_fast(g, rect->width / 3, BOX_TEXTF_H + 3., rect->width / 3, rect->height - BOX_FIJO_H - 3);
		}
		if (!x->jn_nota_ini || !jn_bar_sig_in) {
			jgraphics_set_source_rgba(g, 0.1, 0.1, 0.1, 0.5);
			jgraphics_rectangle_fill_fast(g, 2 * rect->width / 3, BOX_TEXTF_H + 3., rect->width / 3 - 3, rect->height - BOX_FIJO_H - 3);
		}

		//lineas
		object_attr_getjrgba((t_object *)x, ps_elementcolor, &c);
		jgraphics_set_source_jrgba(g, &c);

		jgraphics_move_to(g, 3, BOX_TEXTF_H + 3);
		jgraphics_line_to(g, 3, rect->height - BOX_INFO_H);
		jgraphics_set_line_width(g, 1);
		jgraphics_stroke(g);

		jgraphics_move_to(g, rect->width / 3, BOX_TEXTF_H + 3);
		jgraphics_line_to(g, rect->width / 3, rect->height - BOX_INFO_H);
		jgraphics_set_line_width(g, 1);
		jgraphics_stroke(g);

		jgraphics_move_to(g, 2 * rect->width / 3, BOX_TEXTF_H + 3);
		jgraphics_line_to(g, 2 * rect->width / 3, rect->height - BOX_INFO_H);
		jgraphics_set_line_width(g, 1);
		jgraphics_stroke(g);

		jgraphics_move_to(g, rect->width - 3, BOX_TEXTF_H + 3);
		jgraphics_line_to(g, rect->width - 3, rect->height - BOX_INFO_H);
		jgraphics_set_line_width(g, 1);
		jgraphics_stroke(g);

		//texto cifra
		jgraphics_set_line_width(g, 2.0);
		jgraphics_select_font_face(g, "Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_BOLD);
		jgraphics_set_font_size(g, 8.5);
		if (x->jn_cfra_ini) {
			jgraphics_set_source_rgb(g, 1., 1., 1.);
			if (jn_bar_ant_in && cifra_ant > 0) {
				jgraphics_move_to(g, 35, rect->height - 0.8 * BOX_INFO_H);
				sprintf(text, "%8.5f", cifra_ant);
				jgraphics_show_text(g, text);
			}
			if (jn_bar_act_in && cifra_act > 0) {
				jgraphics_move_to(g, rect->width / 3, rect->height - BOX_INFO_H * 0.8);
				sprintf(text, "%8.5f", cifra_act);
				jgraphics_show_text(g, text);
			}
			if (jn_bar_sig_in && cifra_sig > 0) {
				jgraphics_move_to(g, 2 * rect->width / 3, rect->height - BOX_INFO_H * 0.8);
				sprintf(text, "%8.5f", cifra_sig);
				jgraphics_show_text(g, text);
			}
		}
		else {
			jgraphics_set_source_rgb(g, 1., 0., 0.);
		}
		jgraphics_move_to(g, 3, rect->height - BOX_INFO_H * 0.8);
		jgraphics_show_text(g, "cifra");

		//texto tempo
		if (x->jn_tmpo_ini) {
			jgraphics_set_source_rgb(g, 1., 1., 1.);
			if (jn_bar_ant_in && tmp_type_ant != -1) {
				jgraphics_move_to(g, 35, rect->height - BOX_INFO_H * 0.6);
				if (tmp_type_ant == 0) { sprintf(text, "%8.2f, dl:%8.1f", tmp_tempo_ant, tmp_delay_ant); }
				else if (tmp_type_ant == 1) { sprintf(text, "%8.2f en %8.2f, dl:%8.1f", tmp_tempo_ant, tmp_duracion_ant, tmp_delay_ant); }
				else if (tmp_type_ant == 2) { sprintf(text, "%8.2f en %8.2f, c:%3.2f, dl:%8.1f", tmp_tempo_ant, tmp_duracion_ant, tmp_curva_ant, tmp_delay_ant); }
				jgraphics_show_text(g, text);
			}
			if (jn_bar_act_in && tmp_type_act != -1) {
				jgraphics_move_to(g, rect->width / 3, rect->height - BOX_INFO_H * 0.6);
				if (tmp_type_act == 0) { sprintf(text, "%8.2f, dl:%8.1f", tmp_tempo_act, tmp_delay_act); }
				else if (tmp_type_act == 1) { sprintf(text, "%8.2f en %8.2f, dl:%8.1f", tmp_tempo_act, tmp_duracion_act, tmp_delay_act); }
				else if (tmp_type_act == 2) { sprintf(text, "%8.2f en %8.2f, c:%3.2f, dl:%8.1f", tmp_tempo_act, tmp_duracion_act, tmp_curva_act, tmp_delay_act); }
				jgraphics_show_text(g, text);
			}
			if (jn_bar_sig_in && tmp_type_sig != -1) {
				jgraphics_move_to(g, 2 * rect->width / 3, rect->height - BOX_INFO_H * 0.6);
				if (tmp_type_sig == 0) { sprintf(text, "%8.2f ms, dl:%8.1f", tmp_tempo_sig, tmp_delay_sig); }
				else if (tmp_type_sig == 1) { sprintf(text, "%8.2f ms en %8.2f ms, dl:%8.1f", tmp_tempo_sig, tmp_duracion_sig, tmp_delay_sig); }
				else if (tmp_type_sig == 2) { sprintf(text, "%8.2f ms en %8.2f ms, c:%3.2f, dl:%8.1f", tmp_tempo_sig, tmp_duracion_sig, tmp_curva_sig, tmp_delay_sig); }
				jgraphics_show_text(g, text);
			}
		}
		else {
			jgraphics_set_source_rgb(g, 1., 0., 0.);
		}
		jgraphics_move_to(g, 3, rect->height - BOX_INFO_H * 0.6);
		jgraphics_show_text(g, "tempo");

		//texto goto
		if (jn_bar_sig_in && gt_is_goto_sig && gt_repet_sig > 0) {
			jgraphics_set_source_rgb(g, 0., 1., 0.);
			jgraphics_move_to(g, 0.6667 * rect->width, rect->height - BOX_INFO_H * 0.4);
			jgraphics_show_text(g, "go to");
			jgraphics_set_source_rgb(g, 1., 1., 1.);
			jgraphics_move_to(g, 0.6667 * rect->width + 35, rect->height - BOX_INFO_H * 0.4);
			sprintf(text, "%d \tX%d", gt_tobar_sig, gt_repet_sig);
			jgraphics_show_text(g, text);
		}
		else {
			jgraphics_set_source_rgb(g, 1., 1., 1.);
			jgraphics_move_to(g, 0.6667 * rect->width, rect->height - BOX_INFO_H * 0.4);
			jgraphics_show_text(g, "go to");
		}

		//texto compas
		jgraphics_set_source_rgb(g, 0.8, 0.8, 0.2);
		jgraphics_move_to(g, 0.3334 * rect->width - 40, rect->height - BOX_INFO_H * 0.4);
		jgraphics_show_text(g, "compas");
		jgraphics_move_to(g, 0.3334 * rect->width, rect->height - BOX_INFO_H * 0.4);
		sprintf(text, "%d", jn_bar);
		jgraphics_show_text(g, text);

		if (!is_ini)
			goto no_ini;

		//lineas cifra
		if (x->jn_cfra_ini) {
			if (jn_bar_ant_in && cifra_ant > 0) {
				object_attr_getjrgba((t_object *)x, ps_elementcolor, &c);
				jgraphics_set_source_jrgba(g, &c);
				divs = (long)cifra_ant;
				for (int i = 1; i <= divs; i++) {
					jgraphics_move_to(g, i * rect->width / cifra_ant * 0.3334, 0.85 * (rect->height - BOX_FIJO_H) + BOX_TEXTF_H);
					jgraphics_line_to(g, i * rect->width / cifra_ant * 0.3334, 0.9 * (rect->height - BOX_FIJO_H) + BOX_TEXTF_H);
					jgraphics_set_line_width(g, 1);
					jgraphics_stroke(g);
				}
			}
			if (jn_bar_act_in && cifra_act > 0) {
				object_attr_getjrgba((t_object *)x, ps_elementcolor, &c);
				jgraphics_set_source_jrgba(g, &c);
				divs = (long)cifra_act;
				for (int i = 1; i <= divs; i++) {
					jgraphics_move_to(g, 0.3334 * rect->width * (i / (double)cifra_act + 1), 0.85 * (rect->height - BOX_FIJO_H) + BOX_TEXTF_H);
					jgraphics_line_to(g, 0.3334 * rect->width * (i / (double)cifra_act + 1), 0.9 * (rect->height - BOX_FIJO_H) + BOX_TEXTF_H);
					jgraphics_set_line_width(g, 1);
					jgraphics_stroke(g);
				}
			}
			if (jn_bar_sig_in && cifra_sig > 0) {
				object_attr_getjrgba((t_object *)x, ps_elementcolor, &c);
				jgraphics_set_source_jrgba(g, &c);
				divs = (long)cifra_sig;
				for (int i = 1; i <= divs; i++) {
					jgraphics_move_to(g, 0.3334 * rect->width * (i / (double)cifra_sig + 2), 0.85 * (rect->height - BOX_FIJO_H) + BOX_TEXTF_H);
					jgraphics_line_to(g, 0.3334 * rect->width * (i / (double)cifra_sig + 2), 0.9 * (rect->height - BOX_FIJO_H) + BOX_TEXTF_H);
					jgraphics_set_line_width(g, 1);
					jgraphics_stroke(g);
				}
			}
		}

		//notas
		for (int j = -1; j < 2; j++) {
			if (jn_bar + j < 0 || jn_bar + j >= jn_total_bars) { continue; }
			pbar = x->compases[jn_bar + j];
			total_notas = pbar->total_notas;
			for (int i = 0; i < total_notas; i++) {
				posx = (pbar->hdl_nota[i]->pos_ui.x + 1 + j) * rect->width / 3;
				posy = pbar->hdl_nota[i]->pos_ui.y * (rect->height - BOX_FIJO_H - LADO_CUADRADO) + BOX_TEXTF_H;
				canal = pbar->hdl_nota[i]->canal;

				hue = ((canal * 53) % 360) / 360.0;
				color_sa = hsltorgb(hue, 0.85, 0.3);
				color_nota.red = color_sa.red;
				color_nota.green = color_sa.green;
				color_nota.blue = color_sa.blue;
				color_nota.alpha = 1.;
				jgraphics_set_source_jrgba(g, &color_nota);	// set the color

				jgraphics_move_to(g, posx, posy);
				jgraphics_line_to(g, posx + LADO_CUADRADO, posy + LADO_CUADRADO * 0.5);
				jgraphics_line_to(g, posx, posy + LADO_CUADRADO);
				jgraphics_line_to(g, posx, posy);
				jgraphics_close_path(g);
				jgraphics_fill(g);
			}
		}

		jbox_end_layer((t_object *)x, view, ps_bars);
	}
	jbox_paint_layer((t_object *)x, view, ps_bars, 0., 0.);	// position of the layer

no_ini:
	jbox_end_layer((t_object *)x, view, ps_bars);
	jbox_paint_layer((t_object *)x, view, ps_bars, 0., 0.);
}
void fl_batuta_paint_cursor(t_fl_batuta *x, t_object *view, t_rect *rect)
{
	double pos_cursor;
	t_jrgba c;
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, ps_cursor_layer, rect->width, rect->height);

	if (g) {
		//cursor
		object_attr_getjrgba((t_object *)x, ps_cursorcolor, &c);
		jgraphics_set_source_jrgba(g, &c);

		pos_cursor = (x->pos_cursor_norm * rect->width / 3) + rect->width / 3;
		jgraphics_move_to(g, pos_cursor, BOX_TEXTF_H + 3);
		jgraphics_line_to(g, pos_cursor, rect->height - BOX_FIJO_H + BOX_TEXTF_H);
		jgraphics_set_line_width(g, 1);
		jgraphics_stroke(g);
		jbox_end_layer((t_object *)x, view, ps_cursor_layer);
	}
	jbox_paint_layer((t_object *)x, view, ps_cursor_layer, 0., 0.);	// position of the layer
}
void fl_batuta_paint_textfield(t_fl_batuta *x, t_object *view, t_rect *rect)
{
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, ps_textin_layer, rect->width, rect->height);
	t_jrgba c;

	if (g) {
		// set up matrix
		//jbox_get_rect_for_view((t_object *)x, view, &rect);

		// soft gray background
		object_attr_getjrgba((t_object *)x, ps_elementcolor, &c);
		jgraphics_rectangle(g, 3., 3., rect->width - 6, BOX_TEXTF_H - 3);
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
		jbox_end_layer((t_object *)x, view, ps_textin_layer);
	}
	jbox_paint_layer((t_object *)x, view, ps_textin_layer, 0., 0.);	// position of the layer
}

void fl_batuta_mousemove(t_fl_batuta *x, t_object *patcherview, t_pt pt, long modifiers)
{
	t_rect rect;
	long i, last_over = x->j_overnota;
	x->j_overnota = -1;

	jbox_get_rect_for_view((t_object *)x, patcherview, &rect);

	long total_notas;
	long jn_total_bars = x->jn_total_bars;
	long jn_bar = x->jn_bar;
	fl_bar *pbar;
	double posx, posy, width, height;

	if (x->onoff) { return; }

	if (jn_bar >= 0 && jn_bar < jn_total_bars) {
		total_notas = x->compases[jn_bar]->total_notas;
		pbar = x->compases[jn_bar];
		for (i = 0; i < total_notas; i++) {
			posx = (pbar->hdl_nota[i]->pos_ui.x + 1) / 3;
			posy = (pbar->hdl_nota[i]->pos_ui.y * (rect.height - BOX_FIJO_H - LADO_CUADRADO) + BOX_TEXTF_H) / rect.height;
			width = height = LADO_CUADRADO;
			if (pt.x >= posx * rect.width &&
				pt.y >= 0.5 * (pt.x - posx * rect.width) * height / width + posy * rect.height &&
				pt.y <= -0.5 * (pt.x - posx * rect.width) * height / width + posy * rect.height + height &&
				pt.x >= rect.width / 3 && pt.x <= rect.width * 0.6667) {
				x->j_overnota = i;
				break;
			}
		}
	}

	if (last_over != x->j_overnota) {	// redraw only if it's different
		jbox_redraw((t_jbox *)x);
	}
}

void fl_batuta_mouseleave(t_fl_batuta *x, t_object *patcherview, t_pt pt, long modifiers)
{
	if (x->onoff) { return; }

	x->j_overnota = -1;
	jbox_invalidate_layer((t_object *)x, NULL, ps_bars);
	jbox_redraw((t_jbox *)x);
}

void fl_batuta_mousedown(t_fl_batuta *x, t_object *patcherview, t_pt pt, long modifiers)
{
	if (x->j_overnota >= 0) {
		x->j_selnota = x->j_overnota;
	}

	jbox_invalidate_layer((t_object *)x, NULL, ps_bars);
	jbox_redraw((t_jbox *)x);
}
/*
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