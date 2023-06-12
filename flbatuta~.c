#include "flbatuta~.h"
#include "common/commonsyms.c"	

void ext_main(void *r)
{
	t_class *c;

	common_symbols_init();
	
	c = class_new("flbatuta~", 
		(method)fl_batuta_new, 
		(method)fl_batuta_free, 
		sizeof(t_fl_batuta), (method)NULL, A_GIMME, 0L);

	c->c_flags |= CLASS_FLAG_NEWDICTIONARY;
	jbox_initclass(c, JBOX_TEXTFIELD | JBOX_FONTATTR | JBOX_COLOR); //| JBOX_FIXWIDTH
	class_dspinitjbox(c);

	class_addmethod(c, (method)fl_batuta_dsp64, "dsp64", A_CANT, 0);
	class_addmethod(c, (method)fl_batuta_paint, "paint", A_CANT, 0);
	class_addmethod(c, (method)fl_batuta_assist, "assist", A_CANT, 0);
	class_addmethod(c, (method)jbox_notify, "notify", A_CANT, 0);
	//class_addmethod(c, (method)fl_batuta_mouseleave, "mouseleave", A_CANT, 0);
	//class_addmethod(c, (method)fl_batuta_mousemove, "mousemove", A_CANT, 0);
	//class_addmethod(c, (method)fl_batuta_mousedown, "mousedown", A_CANT, 0);

	class_addmethod(c, (method)fl_batuta_read, "read", A_DEFSYM, 0);
	class_addmethod(c, (method)fl_batuta_write, "write", A_DEFSYM, 0);

	class_addmethod(c, (method)fl_batuta_onoff, "int", A_LONG, 0);
	class_addmethod(c, (method)fl_batuta_nextbar, "in1", A_LONG, 0);

	class_addmethod(c, (method)fl_batuta_new_bar, "bar_new", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_delete_bar, "bar_delete", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_copy_bar, "bar_copy", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_move_bar, "bar_move", A_GIMME, 0);

	class_addmethod(c, (method)fl_batuta_delete_chan, "chan_delete", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_copy_chan, "chan_copy", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_edit_chan, "chan_edit", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_show_range, "chan_show", A_GIMME, 0); //ui
	
	class_addmethod(c, (method)fl_batuta_sel_note, "note_sel", A_GIMME, 0); //ui
	class_addmethod(c, (method)fl_batuta_new_note, "note_new", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_delete_note, "note_delete", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_edit_note, "note_edit", A_GIMME, 0);

	class_addmethod(c, (method)fl_batuta_new_signature, "tsign_new", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_delete_signature, "tsign_delete", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_new_signature, "tsign_edit", A_GIMME, 0);

	class_addmethod(c, (method)fl_batuta_new_tempo, "tempo_new", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_delete_tempo, "tempo_delete", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_new_tempo, "tempo_edit", A_GIMME, 0);

	class_addmethod(c, (method)fl_batuta_new_goto, "goto_new", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_delete_goto, "goto_delete", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_new_goto, "goto_edit", A_GIMME, 0);

	class_addmethod(c, (method)fl_batuta_rec, "rec", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_quantize_chan, "quant", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_human_chan, "human", A_GIMME, 0);

	class_addmethod(c, (method)fl_batuta_info, "info", A_GIMME, 0);

	class_addmethod(c, (method)fl_batuta_key, "key", A_CANT, 0);
	class_addmethod(c, (method)fl_batuta_keyfilter, "keyfilter", A_CANT, 0);
	class_addmethod(c, (method)fl_batuta_enter, "enter", A_CANT, 0);

	CLASS_STICKY_CATEGORY(c, 0, "Color");

	CLASS_ATTR_STYLE_RGBA_PREVIEW(c, "bgcolor", 0, t_fl_batuta, u_background, "Background Color", "rect_fill");
	CLASS_ATTR_BASIC(c, "bgcolor", 0);
	//CLASS_ATTR_STYLE_ALIAS_RGBA_LEGACY(c, "bgcolor", "brgba");
	CLASS_ATTR_STYLE_ALIAS_RGBA_LEGACY(c, "bgcolor", "brgb");
	CLASS_ATTR_LEGACYDEFAULT(c, "bgcolor", "bgcolor", 0, "1. 1. 1. 1.");

	CLASS_ATTR_STYLE_RGBA_PREVIEW(c, "elementcolor", 0, t_fl_batuta, u_lineas, "Off Color", "side_arch");
	CLASS_ATTR_BASIC(c, "elementcolor", 0);
	CLASS_ATTR_STYLE_ALIAS_NOSAVE(c, "elementcolor", "crgb");
	CLASS_ATTR_LEGACYDEFAULT(c, "elementcolor", "elementcolor", 0, "0.9 0.8 0.8 1.");
	
	CLASS_ATTR_STYLE_RGBA_PREVIEW(c, "cursorcolor", 0, t_fl_batuta, u_cursor, "On Color", "side_arch");
	CLASS_ATTR_BASIC(c, "cursorcolor", 0);
	class_attr_stylemap(c, "cursorcolor", "color");
	CLASS_ATTR_LEGACYDEFAULT(c, "cursorcolor", "cursorcolor", 0, "0.2 0.2 0.2 1.");
	CLASS_ATTR_STYLE_ALIAS_NOSAVE(c, "cursorcolor", "frgba");	// ej: I doubt anyone used this but color instead so leaving it as a normal alias (will use class_attr_style_alias if something comes up)

	CLASS_ATTR_RGBA(c, "textcolor", 0, t_fl_batuta, j_textcolor);
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "textcolor", 0, "0.9 0.9 0.9 1.");
	CLASS_ATTR_STYLE_LABEL(c, "textcolor", 0, "rgba", "Text Color");
	CLASS_ATTR_CATEGORY(c, "textcolor", 0, "Color");

	CLASS_STICKY_CATEGORY_CLEAR(c);

	/*
	CLASS_ATTR_RGBA(c, "rectcolor", 0, t_fl_batuta, j_rectcolor);
	CLASS_ATTR_STYLE_LABEL(c, "rectcolor", 0, "rgba", "Rectangle Color");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "rectcolor", 0, "0. 0. 0. 1.");

	CLASS_ATTR_RGBA(c, "overcolor", 0, t_fl_batuta, j_overcolor);
	CLASS_ATTR_STYLE_LABEL(c, "overcolor", 0, "rgba", "Rectangle Over Color");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "overcolor", 0, "1. 1. 1. 1.");
	*/

	CLASS_ATTR_INTRODUCED(c, "elementcolor", 0, "7.0.0");
	
	CLASS_ATTR_DEFAULT(c, "patching_rect", 0, BOX_SIZE_CHAR);

	class_register(CLASS_BOX, c);
	s_fl_batuta_class = c;
}

void *fl_batuta_new(t_symbol *s, long argc, t_atom *argv)
{
	t_fl_batuta *x = NULL;
	t_dictionary *d = NULL;
	t_object *textfield;
	t_max_err err = MAX_ERR_GENERIC;
	long boxflags;

	if (!(d = object_dictionaryarg(argc, argv)))
		return NULL;

	x = (t_fl_batuta *)object_alloc(s_fl_batuta_class);
	if (!x) { return x; }

	boxflags = 0
		| JBOX_DRAWFIRSTIN
		| JBOX_NODRAWBOX
		| JBOX_DRAWINLAST
		| JBOX_TRANSPARENT
		//      | JBOX_NOGROW
		| JBOX_GROWY
		| JBOX_GROWBOTH
		| JBOX_HILITE
		//      | JBOX_BACKGROUND
		| JBOX_DRAWBACKGROUND
		//      | JBOX_NOFLOATINSPECTOR
		//      | JBOX_MOUSEDRAGDELTA
		| JBOX_TEXTFIELD
		;

	err = jbox_new(&x->p_obj.z_box, boxflags, argc, argv);	//	(t_jbox *)x		//	&x->j_box 
	x->p_obj.z_box.b_firstin = (void *)x;

	x->j_selnota = -1;
	x->jn_bar = 0;
	x->jn_total_bars = 0;
	x->jn_total_tsigns = 0;
	x->jn_total_tempos = 0;
	x->jn_total_gotos = 0;
	x->jn_show_all_notes = 1;
	x->jn_chan_min = 0;
	x->jn_chan_max = 0;
	
	x->error_tempo.n_bar = -1;
	x->error_tempo.ms_inicio = -1.f;
	x->error_tempo.ms_durvar = -1.f;
	x->error_tempo.ms_beat = -1.f;
	x->error_tempo.curva = -1.f;

	x->error_tsign.beats = -1.f;
	x->error_tsign.n_bar = -1;

	x->error_goto.cont_rep = -1;
	x->error_goto.n_bar = -1;
	x->error_goto.total_rep = -1;
	x->error_goto.to_bar = -1;

	x->error_bar.psignat_ui = &x->error_tsign;
	x->error_bar.ptempo_ui = &x->error_tempo;
	x->error_bar.pgoto_ui = &x->error_goto;
	x->error_bar.isgoto_ui = -1;
	x->error_bar.total_notas = -1;
	x->error_bar.hdl_nota = NULL;
	x->error_bar.notas = NULL;

	x->text_info = (char *)sysmem_newptr(128 * sizeof(char));
	x->text_size = 128;

	intin(x, 1);

	//mensajes; compas, tempo, dump_info; bang(final)
	x->outlet_bang = bangout((t_object *)x);			//bangfinal
	x->outlet_mevent = outlet_new((t_object *)x, NULL);	//evento
	x->outlet_ibar = intout((t_object *)x);				//compas
	x->outlet_fcifra = floatout((t_object *)x);			//cifra

	//textfield
	textfield = jbox_get_textfield((t_object *)x);
	if (textfield) {
		textfield_set_editonclick(textfield, 1);			// set it to 0 if you don't want user to edit it in lock mode
		textfield_set_textmargins(textfield, 3, 3, 3, 3);	// margin on each side
		textfield_set_textcolor(textfield, &x->j_textcolor);// textcolor
	}
	attr_dictionary_process((t_object *)x, d); //si se definió atributo

	dsp_setupjbox((t_pxjbox *)x, 0);
	outlet_new((t_object *)x, "signal");	//tempo
	outlet_new((t_object *)x, "signal");	//compas norm
	outlet_new((t_object *)x, "signal");	//beat norm
	x->p_obj.z_misc |= Z_NO_INPLACE;

	/*--------------------------------------------------------------*/
	//editar
	x->isplaying = 0;
	x->isloading = 0;
	x->isediting = 0;
	
	x->largo_texto = 0;
	x->max_buf_len = MAX_BUF_LEN;

	//crear lista tsigns
	x->l_tsigns = linklist_new();
	linklist_flags(x->l_tsigns, OBJ_FLAG_MEMORY);
	//crear lista tempos
	x->l_tempos = linklist_new();
	linklist_flags(x->l_tempos, OBJ_FLAG_MEMORY);
	//crear lista gotos
	x->l_gotos = linklist_new();
	linklist_flags(x->l_gotos, OBJ_FLAG_MEMORY);
	//crear lista de bars
	x->l_bars = linklist_new();
	linklist_flags(x->l_bars, OBJ_FLAG_MEMORY);

	//------------------------------------------------------reproducir
	x->ms_beat = 500;
	x->n_bar = 0;
	x->negras = 4;
	x->total_tsigns = 0;
	x->total_tempos = 0;
	x->total_gotos = 0;
	x->total_bars = 0;
	x->total_notes_out = 0;

	x->task_new_idx = 0;
	x->task_out_bar = 0;
	x->task_end_flag = 0;

	x->old_msbeat = 500;
	x->new_msbeat = 500;
	x->task_tempo = 0;
	x->cont_tempo = 500;
	x->durac_dtempo = 0;
	x->curva_dtempo = 1.;
	x->delay_dtempo = 0;

	x->index_tempo = 0;
	x->index_cifra = 0;
	x->index_goto = 0;
	x->index_nota = 0;
	x->index_compas = 0;

	x->tempos = (fl_tempo **)sysmem_newptr(sizeof(fl_tempo *));
	x->gotos = (fl_goto **)sysmem_newptr(sizeof(fl_goto *));
	x->tsigns = (fl_tsign **)sysmem_newptr(sizeof(fl_tsign *));
	x->notes_out = (fl_note **)sysmem_newptr(MAX_NOTES_OUT * sizeof(fl_note *));
	x->bars = (fl_bar **)sysmem_newptr(sizeof(fl_bar *));

	x->rec_data = (fl_note_rec *)sysmem_newptr(NELEMS_REC * sizeof(fl_note_rec));
	for (int i = 0; i < NELEMS_REC; i++) {
		x->rec_data[i].ap = (t_atom *)sysmem_newptr(sizeof(t_atom));
		x->rec_data[i].ac = 0;
		x->rec_data[i].bar = -1;
		x->rec_data[i].b_start = -1.f;
	}
	x->total_rec = 0;
	x->index_elem_rec = 0;
	x->dirty_rec = 0;

	x->parsed_atoms = (t_atom *)sysmem_newptr(128 * sizeof(t_atom));

	//gotos
	x->next_bar = 0;
	x->next_bar_dirty = 0;

	//clocks
	x->bang_clock = clock_new((t_object *)x, (method)fl_batuta_bang);
	x->outmes_clock = clock_new((t_object *)x, (method)fl_batuta_outmensaje);
	x->outbar_clock = clock_new((t_object *)x, (method)fl_batuta_outcompas);
	x->outcifra_clock = clock_new((t_object *)x, (method)fl_batuta_outcifra);
	x->cursor_clock = clock_new((t_object *)x, (method)fl_batuta_tick);

	srand((unsigned int)clock());

	do_add_bar(x, 0);
	do_add_tempo(x, 0, 0.f, 500.f, 0., 0.);
	do_add_signature(x, 0, 4.);
	fl_batuta_update_notes(x);
	fl_batuta_update_tempos(x);
	fl_batuta_update_signatures(x);

	jbox_ready((t_jbox *)x); //non signal ui: jbox_ready(&x->j_box);
	
	fl_batuta_update_uibar(x);
	fl_batuta_update_uitempo(x);
	
	return x;
}
void fl_batuta_assist(t_fl_batuta *x, void *b, long msg, long arg, char *dst)
{
	if (msg == ASSIST_INLET) {
		switch (arg) {
			/* inputs */
		case I_ONOFF: sprintf(dst, "(int) on/off");
			break;
		case I_NEXTBAR: sprintf(dst, "(int) bar index(edit)/next bar(play)");
			break;
		}
	}
	else if (msg == ASSIST_OUTLET) {
		switch (arg) {
			/* outputs */
		case O_BEATSIG: sprintf(dst, "(sig~) beat (norm)");
			break;
		case O_BARSIG: sprintf(dst, "(sig~) bar (norm)");
			break;
		case O_TEMPO: sprintf(dst, "(sig~) ms beat (tempo)");
			break;
		case O_CIFRA: sprintf(dst, "(float) time signature");
			break;
		case O_COMPAS: sprintf(dst, "(int) bar index");
			break;
		case O_OUTPUT: sprintf(dst, "(message) chan, info...");
			break;
		case O_BANG: sprintf(dst, "(bang) final flag");
			break;
		}
	}
}

/* memory -------------------------------------------------------------------- */
void fl_batuta_free(t_fl_batuta *x)
{
	long total_bars;
	fl_bar *pbar;
	long total_notas;
	fl_note *pnota;
	
	dsp_freejbox((t_pxjbox *)x);

	for (int i = 0; i < NELEMS_REC; i++) {
		sysmem_freeptr(x->rec_data[i].ap);
	}
	sysmem_freeptr(x->rec_data);

	total_bars = (long)linklist_getsize(x->l_bars);
	for (int i = 0; i < total_bars; i++) {
		pbar = linklist_getindex(x->l_bars, i);
		total_notas = (long)linklist_getsize(pbar->notas);
		for (int k = 0; k < total_notas; k++) {
			pnota = linklist_getindex(pbar->notas, k);
			sysmem_freeptr(pnota->pnota);
		}
		object_free(pbar->notas);
		if (!pbar->hdl_nota) {
			sysmem_freeptr(pbar->hdl_nota);
		}
	}
	object_free(x->l_bars);
		
	object_free(x->l_tempos);
	object_free(x->l_tsigns);
	object_free(x->l_gotos);

	sysmem_freeptr(x->tempos);
	sysmem_freeptr(x->tsigns);
	sysmem_freeptr(x->gotos);
	sysmem_freeptr(x->notes_out);
	sysmem_freeptr(x->bars);

	object_free(x->bang_clock);
	object_free(x->outmes_clock);
	object_free(x->outbar_clock);
	object_free(x->outcifra_clock); 
	object_free(x->cursor_clock);

	sysmem_freeptr(x->text_info);
	
	sysmem_freeptr(x->parsed_atoms);

	jbox_free((t_jbox *)x);
}

/* console ------------------------------------------------------------------- */
void fl_batuta_info(t_fl_batuta  *x, t_symbol *s, long argc, t_atom *argv)
{
	t_atom *ap = argv;
	long ac = argc;
	long n_bar;
	long n_canal;
	long total_bars;
	fl_bar *pbar;
	char *texto;
	long texto_len = 128;
	fl_note *pnota;
	long total_notas;
	long total_tsigns;
	long total_tempos;
	long total_gotos;
	fl_tempo *ptempo;
	fl_tsign *pcifra;
	fl_goto *pgoto;

	if (!ac) {
		object_post((t_object *)x, "bar:"
			"\nbar_new [0arg(i)]"
			"\nbar_new [1arg(i)]\tbar index"
			"\nbar_delete [1arg(i)]\tbar index"
			"\nbar_copy [2arg(ii)]\tbar index, target bar index"
			"\nbar_move [2arg(ii)]\tbar index, target bar index");
		object_post((t_object *)x, "channel:"
			"\nchan_edit [3arg(iii)]\tbar index, chan, new chan"
			"\nchan_copy [3arg(iii)]\tbar index, chan, target bar index"
			"\nchan_delete [2arg(ii)]\tbar index, chan");
		object_post((t_object *)x, "note:"
			"\nnote_new [min4arg(iif...)]\tbar index, chan, beat start, info..."
			"\nnote_edit [min4arg(iif...)]\tbar index, chan, note index, info..."
			"\nbar_delete [3arg(iif)]\tbar index, chan, note index");
		object_post((t_object *)x, "tempo:"
			"\ntempo_new [2 arg(iffff)]\tbar index, period ms beat"
			"\ntempo_new [3 arg(iffff)]\tbar index, period ms beat, delay ms"
			"\ntempo_new [4 arg(iffff)]\tbar index, period ms beat, delay ms, change duration"
			"\ntempo_new [5 arg(iffff)]\tbar index, period ms beat, delay ms, change duration, curve"
			"\ntempo_edit [2-5 arg(iffff)]\tidem as above"
			"\ntempo_delete [1arg(i)]\tbar index");
		object_post((t_object *)x, "time signature:"
			"\ntsign_new [3arg(ifi)]\tbar index, beats per bar"
			"\ntsign_edit [3arg(ifi)]\tidem as above"
			"\ntsign_delete [1arg(i)]\tbar index");
		object_post((t_object *)x, "goto:"
			"\ngoto_new [3arg(iii)]\tbar index, target bar index, repetitions"
			"\ngoto_edit [3arg(iii)]\tidem as above"
			"\ngoto_delete [1arg(i)]\tbar index");
		object_post((t_object *)x, "others:"
			"\nrec [2arg min(i...)]\tchan, info..."
			"\nquant [3arg (iii)]\tbar index, chan, division of quarter"
			"\nhuman [4arg (iiff)]\tbar index, chan, (+/-)fixed range(%%beat), (+/-)random range(%%beat)");
		return;
	}

	if (atom_gettype(ap) != A_SYM) {
		object_error((t_object *)x, "valid args: goto, tempo, tsign, note, bar");
		return;
	}

	if (ac == 1) {
		if (atom_getsym(ap) == gensym("bar")) {
			total_bars = (long)linklist_getsize(x->l_bars);
			object_post((t_object *)x, "total bars: %d", total_bars);
		}
		else if (atom_getsym(ap) == gensym("tempo")) {
			total_tempos = (long)linklist_getsize(x->l_tempos);
			object_post((t_object *)x, "total tempos: %d", total_tempos);
			for (int i = 0; i < total_tempos; i++) {
				ptempo = linklist_getindex(x->l_tempos, i);
				object_post((t_object *)x,
					"tempo: bar:%d, delay:%.2f, ms tempo:%.2f, change dur ms:%.2f, curve:%.2f",
					ptempo->n_bar, ptempo->ms_inicio, ptempo->ms_beat, ptempo->ms_durvar, ptempo->curva);
			}
		}
		else if (atom_getsym(ap) == gensym("tsign")) {
			total_tsigns = (long)linklist_getsize(x->l_tsigns);
			object_post((t_object *)x, "total time signatures: %d", total_tsigns);
			for (int i = 0; i < total_tsigns; i++) {
				pcifra = linklist_getindex(x->l_tsigns, i);
				object_post((t_object *)x, "tsign: bar:%d, beats:%.2f", pcifra->n_bar, pcifra->beats);
			}
		}
		else if (atom_getsym(ap) == gensym("goto")) {
			total_gotos = (long)linklist_getsize(x->l_gotos);
			object_post((t_object *)x, "total gotos: %d", total_gotos);
			for (int i = 0; i < total_gotos; i++) {
				pgoto = linklist_getindex(x->l_gotos, i);
				object_post((t_object *)x, "goto: bar:%d, goto:%d, reps:%d", pgoto->n_bar, pgoto->to_bar, pgoto->total_rep);
			}
		}
	}
	else if (ac == 2) {
		if (atom_getsym(ap) == gensym("chan")) {
			if (atom_gettype(ap + 1) != A_LONG) { object_warn((t_object *)x, "bar index must be a number (integer)"); return; }
			n_bar = (long)atom_getlong(ap + 1);
			total_bars = (long)linklist_getsize(x->l_bars);
			if (n_bar < 0 || n_bar >= total_bars) { object_warn((t_object *)x, "bar index must be 0 or positive less than the total of bars"); return; }
			pbar = linklist_getindex(x->l_bars, n_bar);
			total_notas = (long)linklist_getsize(pbar->notas);
			for (int i = 0; i < total_notas; i++) {
				pnota = linklist_getindex(pbar->notas, i);
				n_canal = pnota->canal;
				object_post((t_object *)x, "chan: %d", n_canal);
			}
		}
	}
	else if (ac == 3) {
		texto = (char *)sysmem_newptr(LARGO_MAX_LINEA * sizeof(char));
		if (atom_getsym(ap) == gensym("note")) {
			if (atom_gettype(ap + 1) != A_LONG) { object_warn((t_object *)x, "bar index must be a number (integer)"); return; }
			if (atom_gettype(ap + 2) != A_LONG) { object_warn((t_object *)x, "channel must be a number (integer)"); return; }
			n_bar = (long)atom_getlong(ap + 1);
			n_canal = (long)atom_getlong(ap + 2);
			total_bars = (long)linklist_getsize(x->l_bars);
			if (n_bar < 0 || n_bar >= total_bars) { object_warn((t_object *)x, "bar index must be 0 or a positive less than the total of bars"); return; }
			pbar = linklist_getindex(x->l_bars, n_bar);

			total_notas = (long)linklist_getsize(pbar->notas);
			if (!total_notas) { object_warn((t_object *)x, "bar %d doesn't have any notes", n_bar); return; }
			object_post((t_object *)x, "bar %d, chan %d --------------------", n_bar, n_canal);
			
			texto = (char *)sysmem_newptr(texto_len * sizeof(char));
			if (texto) {
				for (int i = 0; i < total_notas; i++) {
					pnota = linklist_getindex(pbar->notas, i);
					atom_gettext((pnota->cnota) - 1, (pnota->pnota) + 1, &texto_len, &texto, 0);
					object_post((t_object *)x, "start:%.2f, info:%s", pnota->b_inicio, texto);
				}
				sysmem_freeptr(texto);
			}
			object_post((t_object *)x, "-----------------------------------------");
		}
		sysmem_freeptr(texto);
	}
}

/* outlets ------------------------------------------------------------------- */
void fl_batuta_bang(t_fl_batuta  *x)
{
	outlet_bang(x->outlet_bang);
}
void fl_batuta_outmensaje(t_fl_batuta  *x)
{
	fl_note **hnotas = x->notes_out;
	long total_notes_out = x->total_notes_out;
	
	for (int i = 0; i < total_notes_out; i++) {
		outlet_list(x->outlet_mevent, NULL, (short)hnotas[i]->cnota, hnotas[i]->pnota);
	}
}
void fl_batuta_outcompas(t_fl_batuta  *x)
{
	long n_bar = x->n_bar;

	outlet_int(x->outlet_ibar, n_bar);

	x->jn_bar = n_bar;
	jbox_invalidate_layer((t_object*)x, NULL, gensym("bars_layer"));
	jbox_invalidate_layer((t_object*)x, NULL, gensym("notes_layer"));
	jbox_invalidate_layer((t_object*)x, NULL, gensym("info_layer"));
	jbox_redraw((t_jbox *)x);
}
void fl_batuta_outcifra(t_fl_batuta  *x)
{
	outlet_float(x->outlet_fcifra, x->negras);
}
