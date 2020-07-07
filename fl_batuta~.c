#include "fl_batuta~.h"

void ext_main(void *r)
{
	t_class *c;

	common_symbols_init();
	fl_batuta_symbols_init();

	c = class_new("fl_batuta~", 
		(method)fl_batuta_new, 
		(method)fl_batuta_free, 
		sizeof(t_fl_batuta), 0L, A_GIMME, 0);

	c->c_flags |= CLASS_FLAG_NEWDICTIONARY;
	jbox_initclass(c, JBOX_TEXTFIELD | JBOX_FONTATTR | JBOX_COLOR); //| JBOX_FIXWIDTH
	class_dspinitjbox(c);

	class_addmethod(c, (method)fl_batuta_dsp64, "dsp64", A_CANT, 0);
	class_addmethod(c, (method)fl_batuta_paint, "paint", A_CANT, 0);
	class_addmethod(c, (method)fl_batuta_assist, "assist", A_CANT, 0);
	class_addmethod(c, (method)jbox_notify, "notify", A_CANT, 0);
	class_addmethod(c, (method)fl_batuta_mouseleave, "mouseleave", A_CANT, 0);
	class_addmethod(c, (method)fl_batuta_mousemove, "mousemove", A_CANT, 0);
	class_addmethod(c, (method)fl_batuta_mousedown, "mousedown", A_CANT, 0);

	class_addmethod(c, (method)fl_batuta_read, "read", A_DEFSYM, 0);
	class_addmethod(c, (method)fl_batuta_write, "write", A_DEFSYM, 0);

	class_addmethod(c, (method)fl_batuta_onoff, "int", A_LONG, 0);
	class_addmethod(c, (method)fl_batuta_nextbar, "in1", A_LONG, 0);

	class_addmethod(c, (method)fl_batuta_nuevo_compas, "nuevo_compas", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_borrar_compas, "borrar_compas", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_copiar_compas, "copiar_compas", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_mover_compas, "mover_compas", A_GIMME, 0);

	class_addmethod(c, (method)fl_batuta_borrar_canal, "borrar_canal", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_copiar_canal, "copiar_canal", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_editar_canal, "editar_canal", A_GIMME, 0);

	class_addmethod(c, (method)fl_batuta_nueva_nota, "nueva_nota", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_borrar_nota, "borrar_nota", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_editar_nota, "editar_nota", A_GIMME, 0);

	class_addmethod(c, (method)fl_batuta_nueva_cifra, "nueva_cifra", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_borrar_cifra, "borrar_cifra", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_nueva_cifra, "editar_cifra", A_GIMME, 0);

	class_addmethod(c, (method)fl_batuta_nuevo_tempo, "nuevo_tempo", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_borrar_tempo, "borrar_tempo", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_nuevo_tempo, "editar_tempo", A_GIMME, 0);

	class_addmethod(c, (method)fl_batuta_nuevo_goto, "nuevo_goto", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_borrar_goto, "borrar_goto", A_GIMME, 0);
	class_addmethod(c, (method)fl_batuta_nuevo_goto, "editar_goto", A_GIMME, 0);

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

void fl_batuta_symbols_init(void)
{
	ps_elementcolor = gensym("elementcolor");
	ps_cursorcolor = gensym("cursorcolor");

	ps_cursor_layer = gensym("cursor_layer");
	ps_textin_layer = gensym("textin_layer");
	ps_bars = gensym("compases");

	ps_cat_compas = gensym("compas");
	ps_cat_canal = gensym("canal");
	ps_cat_canal = gensym("canal");
	ps_cat_nota = gensym("nota");
	ps_cat_cifra = gensym("cifra");
	ps_cat_goton = gensym("goton");
	ps_cat_tempo = gensym("tempo");
	ps_subc_nota_info = gensym("info");
	ps_subc_nota_canal = gensym("canal");
	ps_subc_nota_inicio = gensym("inicio");

	ps_acc_nuevo = gensym("-n");
	ps_acc_editar = gensym("-e");
	ps_acc_borrar = gensym("-x");
	ps_acc_copiar = gensym("-c");
	ps_acc_mover = gensym("-m");
	ps_acc_info = gensym("-i");
	ps_acc_human = gensym("-h");
	ps_acc_quantize = gensym("-q");
	ps_acc_repi = gensym("{");
	ps_acc_repf = gensym("}");
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

	x->j_overnota = -1;
	x->j_selnota = -1;
	x->jn_bar = 0;
	x->jn_total_bars = 0;
	x->jn_cfra_ini = 0;
	x->jn_tmpo_ini = 0;
	x->jn_nota_ini = 0;
	x->pbar_error = (fl_bar *)sysmem_newptr(sizeof(fl_bar));
	x->pbar_error->cifra_ui = -1;
	x->pbar_error->tempo_ui.type = -1;
	x->pbar_error->pgoto_ui = NULL;
	x->pbar_error->isgoto_ui = 0;
	x->pbar_error->total_notas = 0;
	x->pbar_error->hdl_nota = NULL;

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
	x->onoff = 0;
	x->largo_texto = 0;

	//crear lista cifras
	x->l_cifras = linklist_new();
	linklist_flags(x->l_cifras, OBJ_FLAG_MEMORY);
	//crear lista tempos
	x->l_tempos = linklist_new();
	linklist_flags(x->l_tempos, OBJ_FLAG_MEMORY);
	//crear lista gotos
	x->l_gotos = linklist_new();
	linklist_flags(x->l_gotos, OBJ_FLAG_MEMORY);
	//crear lista de compases
	x->l_bars = linklist_new();
	linklist_flags(x->l_bars, OBJ_FLAG_MEMORY);

	//------------------------------------------------------reproducir
	x->tempo_dirty = 1;
	x->cifra_dirty = 1;
	x->nota_dirty = 1;

	x->ms_beat = 500;
	x->n_bar = 0;
	x->negras = 4;
	x->total_cifras = 0;
	x->total_tempos = 0;
	x->total_gotos = 0;
	x->total_bars = 0;
	x->total_notas_out = 0;

	x->old_msbeat = 500;
	x->new_msbeat = 500;
	x->dtempo_busy = 0;
	x->cont_tempo = 500;
	x->durac_dtempo = 0;
	x->curva_dtempo = 0.5;
	x->type_dtempo = 0;
	x->delay_dtempo = 0;

	x->index_tempo = 0;
	x->index_cifra = 0;
	x->index_goto = 0;
	x->index_nota = 0;
	x->index_compas = 0;

	x->tempos = (fl_tempo **)sysmem_newptr(sizeof(fl_tempo *));
	x->gotos = (fl_goto **)sysmem_newptr(sizeof(fl_goto *));
	x->cifras = (fl_cifra **)sysmem_newptr(sizeof(fl_cifra *));
	x->notas_out = (fl_nota **)sysmem_newptr(MAX_NOTAS_OUT * sizeof(fl_nota *));
	x->compases = (fl_bar **)sysmem_newptr(sizeof(fl_bar));

	x->rec_data = (fl_nota_rec *)sysmem_newptr(NELEMS_REC * sizeof(fl_nota_rec));
	for (int i = 0; i < NELEMS_REC; i++) {
		x->rec_data[i].ap = (t_atom *)sysmem_newptr(sizeof(t_atom));
		x->rec_data[i].ac = 0;
	}
	x->total_rec = 0;
	x->index_elem_rec = 0;
	x->dirty_rec = 0;

	x->parsed_atoms = (t_atom *)sysmem_newptr(512 * sizeof(t_atom));

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

	x->startclock = false;

	jbox_ready((t_jbox *)x);
	//jbox_ready(&x->p_obj);

	return x;
}
void fl_batuta_assist(t_fl_batuta *x, void *b, long msg, long arg, char *dst)
{
	if (msg == ASSIST_INLET) {
		switch (arg) {
			/* inputs */
		case I_ONOFF: sprintf(dst, "(int) on/off; (mensaje)");
			break;
		case I_NEXTBAR: sprintf(dst, "compas siguiente");
			break;
		}
	}
	else if (msg == ASSIST_OUTLET) {
		switch (arg) {
			/* outputs */
		case O_BEATSIG: sprintf(dst, "(sig~) beat (norm)");
			break;
		case O_BARSIG: sprintf(dst, "(sig~) compas (norm)");
			break;
		case O_TEMPO: sprintf(dst, "(sig~) ms beat (tempo)");
			break;
		case O_CIFRA: sprintf(dst, "(float) cifra");
			break;
		case O_COMPAS: sprintf(dst, "(int) compas");
			break;
		case O_OUTPUT: sprintf(dst, "(mensaje) canal, evento");
			break;
		case O_BANG: sprintf(dst, "(bang) flag final");
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
	fl_nota *pnota;
	
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
	object_free(x->l_cifras);
	object_free(x->l_gotos);

	sysmem_freeptr(x->tempos);
	sysmem_freeptr(x->cifras);
	sysmem_freeptr(x->gotos);
	sysmem_freeptr(x->notas_out);
	sysmem_freeptr(x->compases);

	object_free(x->bang_clock);
	object_free(x->outmes_clock);
	object_free(x->outbar_clock);
	object_free(x->outcifra_clock); 
	object_free(x->cursor_clock);

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
	long texto_len = 0;
	fl_nota *pnota;
	long total_notas;
	long total_cifras;
	long total_tempos;
	long total_compases;
	long total_gotos;
	fl_tempo *ptempo;
	fl_cifra *pcifra;
	fl_goto *pgoto;

	if (!ac) {
		object_post((t_object *)x, "formato compas:"
			"\nnuevo_compas 0-1arg(i):\tagrega al final o en index"
			"\nborrar_compas 1arg(i):\tborrar en index"
			"\neditar_compas 2arg(ii):\tindex origen, index destino"
			"\nmover_compas 2arg(ii):\tindex origen, index destino");
		object_post((t_object *)x, "formato canal:"
			"\neditar_canal 3arg(iii):\tn compas; n canal; n canal editado"
			"\ncopiar_canal 3arg(iii):\tn compas; n canal; n compas destino"
			"\nborrar_canal 2arg(ii):\tn compas; n canal");
		object_post((t_object *)x, "formato nota:"
			"\nnueva_nota >4arg(iif...):\tn compas; beat inicio; n canal dest; info..."
			"\neditar_nota >4arg(iif...):\tn compas; beat inicio; n canal dest; info..."
			"\nborrar_nota 3arg(iif):\tn compas; n canal dest; index nota");
		object_post((t_object *)x, "formato tempo:"
			"\nnuevo_tempo 3 a 5 arg(iffff):\tn compas; ms inicio; ms beat; (durac dtempo; curva)"
			"\neditar_tempo 3 a 5 arg(iffff):\tn compas; ms inicio; ms beat; (durac dtempo; curva)"
			"\nborrar_tempo 1arg(i): n compas");
		object_post((t_object *)x, "formato cifra:"
			"\nnueva_cifra 3arg(ifi):\tn compas; numerador; denominador"
			"\neditar_cifra 3arg(ifi):\tn compas; numerador; denominador"
			"\nborrar_cifra 1arg(i):\tn compas");
		object_post((t_object *)x, "formato goto:"
			"\nnuevo_goto 3arg(iii):\tn compas origen; n compas destino; repeticiones"
			"\neditar_goto 3arg(iii):\tn compas origen; n compas destino; repeticiones"
			"\nborrar_goto 1arg(i):\tn compas");
		object_post((t_object *)x, "otros:"
			"\nrec: 2arg min(i...):\tcanal, info..."
			"\nquant: 3arg (iii):\tcompas, canal, divisiones de negra"
			"\nhuman: 4arg (iiff):\tcompas, canal, (+/-)margen fijo(%%beat), (+/-)margen aleatorio(%%beat)");
		return;
	}

	if (atom_gettype(ap) != A_SYM) {
		object_error((t_object *)x, "se acepta: goto, tempo, cifra, nota, compas");
		return;
	}

	if (ac == 1) {
		if (atom_getsym(ap) == gensym("compas")) {
			total_compases = (long)linklist_getsize(x->l_bars);
			object_post((t_object *)x, "total compases: %d", total_compases);
		}
		else if (atom_getsym(ap) == gensym("tempo")) {
			total_tempos = (long)linklist_getsize(x->l_tempos);
			object_post((t_object *)x, "total tempos: %d", total_tempos);
			for (int i = 0; i < total_tempos; i++) {
				ptempo = linklist_getindex(x->l_tempos, i);
				if (ptempo->type == 0) {
					object_post((t_object *)x,
						"tempo: compas:%d, delay inicio:%.2f, ms tempo:%.2f",
						ptempo->n_bar, ptempo->ms_inicio, ptempo->ms_beat);
				}
				else if (ptempo->type == 1) {
					object_post((t_object *)x,
						"tempo: compas:%d, delay inicio:%.2f, ms tempo:%.2f, dur cambio ms:%.2f",
						ptempo->n_bar, ptempo->ms_inicio, ptempo->ms_beat, ptempo->ms_durvar);
				}
				else if (ptempo->type == 2) {
					object_post((t_object *)x,
						"tempo: compas:%d, delay inicio:%.2f, ms tempo:%.2f, dur cambio ms:%.2f, curva:%.2f",
						ptempo->n_bar, ptempo->ms_inicio, ptempo->ms_beat, ptempo->ms_durvar, ptempo->curva);
				}
			}
		}
		else if (atom_getsym(ap) == gensym("cifra")) {
			total_cifras = (long)linklist_getsize(x->l_cifras);
			object_post((t_object *)x, "total cifras: %d", total_cifras);
			for (int i = 0; i < total_cifras; i++) {
				pcifra = linklist_getindex(x->l_cifras, i);
				object_post((t_object *)x, "cifra: compas:%d, negras:%.2f", pcifra->n_bar, pcifra->negras);
			}
		}
		else if (atom_getsym(ap) == gensym("goto")) {
			total_gotos = (long)linklist_getsize(x->l_gotos);
			object_post((t_object *)x, "total gotos: %d", total_gotos);
			for (int i = 0; i < total_gotos; i++) {
				pgoto = linklist_getindex(x->l_gotos, i);
				object_post((t_object *)x, "goto: compas:%d, goto:%d, veces:%d", pgoto->n_bar, pgoto->to_bar, pgoto->total_rep);
			}
		}
	}
	else if (ac == 2) {
		if (atom_getsym(ap) == gensym("canal")) {
			if (atom_gettype(ap + 1) != A_LONG) { object_post((t_object *)x, "n compas debe ser long"); return; }
			n_bar = (long)atom_getlong(ap + 1);
			total_bars = (long)linklist_getsize(x->l_bars);
			if (n_bar >= total_bars) { object_post((t_object *)x, "n compas es mayor que total de compases"); return; }
			pbar = linklist_getindex(x->l_bars, n_bar);
			total_notas = (long)linklist_getsize(pbar->notas);

			for (int i = 0; i < total_notas; i++) {
				pnota = linklist_getindex(pbar->notas, i);
				n_canal = pnota->canal;
				object_post((t_object *)x, "canal: %d", n_canal);
			}
		}
	}
	else if (ac == 3) {
		texto = (char *)sysmem_newptr(LARGO_MAX_LINEA * sizeof(char));
		if (atom_getsym(ap) == gensym("nota")) {
			if (atom_gettype(ap + 1) != A_LONG) { object_post((t_object *)x, "n compas debe ser long"); return; }
			if (atom_gettype(ap + 2) != A_LONG) { object_post((t_object *)x, "n canal debe ser long"); return; }
			n_bar = (long)atom_getlong(ap + 1);
			n_canal = (long)atom_getlong(ap + 2);
			total_bars = (long)linklist_getsize(x->l_bars);
			if (n_bar >= total_bars) { object_post((t_object *)x, "n compas es mayor que total de compases"); return; }
			pbar = linklist_getindex(x->l_bars, n_bar);

			object_post((t_object *)x, "compas %d, canal %d:", n_bar, n_canal);
			total_notas = (long)linklist_getsize(pbar->notas);
			for (int i = 0; i < total_notas; i++) {
				pnota = linklist_getindex(pbar->notas, i);
				my_gettext((pnota->cnota) - 1, (pnota->pnota) + 1, &texto_len, &texto, 0);
				object_post((t_object *)x, "inicio:%.2f, info:%s", pnota->b_inicio, texto);
			}
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
	fl_nota **hnotas = x->notas_out;
	long total_notas_out = x->total_notas_out;
	
	for (int i = 0; i < total_notas_out; i++) {
		outlet_list(x->outlet_mevent, NULL, (short)hnotas[i]->cnota, hnotas[i]->pnota);
	}
}
void fl_batuta_outcompas(t_fl_batuta  *x)
{
	long n_bar = x->n_bar;

	outlet_int(x->outlet_ibar, n_bar);

	x->jn_bar = n_bar;
	jbox_invalidate_layer((t_object*)x, NULL, ps_bars);
	jbox_redraw((t_jbox *)x);
}
void fl_batuta_outcifra(t_fl_batuta  *x)
{
	outlet_float(x->outlet_fcifra, x->negras);
}
