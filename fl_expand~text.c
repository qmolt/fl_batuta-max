#include "fl_batuta~.h"

long fl_batuta_key(t_fl_batuta *x, t_object *patcherview, long keycode, long modifiers, long textcharacter)
{
	char buff[256];
	buff[0] = (char)textcharacter;
	buff[1] = 0;
	object_method(patcherview, gensym("insertboxtext"), x, buff);
	jbox_redraw((t_jbox *)x);

	return 1;
}

long fl_batuta_keyfilter(t_fl_batuta *x, t_object *patcherview, long *keycode, long *modifiers, long *textcharacter)
{
	t_atom arv;
	long k = *keycode;

	if (k == JKEY_TAB || k == JKEY_ENTER || k == JKEY_ESC) {//|| k == JKEY_RETURN
		object_method_typed(patcherview, gensym("endeditbox"), 0, NULL, &arv);
		return 0;	// don't pass those keys to uitextfield
	}

	return 1;
}

void fl_batuta_enter(t_fl_batuta *x)
{
	t_max_err err = MAX_ERR_NONE;
	long ac = 128;
	t_atom *av = x->parsed_atoms;
	t_symbol *param, *accion, *subcat;
	long size = 0;
	char *text = NULL;
	t_object *textfield = jbox_get_textfield((t_object *)x);

	long total_bars, bar_act, bar_dest;
	long chan_orig, chan_final, chan_index;
	long note_index;
	long rep;
	float beats;
	long j_selnota;
	short tipo;
	float mstempo, variacion, inicio, curva;
	long index_repf, index_string, subdiv;
	float beat_acum, beat;
	char *str_lista;
	float rango_fijo, rango_random, division;

	object_method(textfield, gensym("gettextptr"), &text, &size);
	if (!size) {return;}

	err = atom_setparse(&ac, &av, text);
	if (err) { object_error((t_object *)x, "couldn't get command"); return; }

	if (ac == 1) {
		if (atom_gettype(av) == A_SYM) {
			accion = atom_getsym(av);
			if (accion == gensym("-i")) {
				object_post((t_object *)x,
					"bars (bar, b)\n"
					"-n [new bar]\n"
					"-x [delete bar]\n"
					"-m (n) [move bar to (int)]\n"
					"-c (n) [copy bar and paste it in (int)]");
				object_post((t_object *)x,
					"channel (chan, ch)\n"
					"-w all / -w (n) / -w (min max) [show all/n/range channels]"
					"-e (n) / -e + (sel note ch) [edit channel to (int)]\n"
					"-x (n) / -x + (sel note ch) [delete whole channel]\n"
					"-c (n) / -c + (sel note ch) [copy channel to (int) bar]\n"
					"-h (n) (f) (r) / -h (f) (r) + (sel note ch) [changes +/- (f)ixed%% and (r)andom%% from beat start]\n"
					"-q (n) (d) / -q (d) + (sel note ch) [quantize to (d)ivision of beat (quarter note)]");
				object_post((t_object *)x,
					"notes (note, n)\n"
					"-s (ch) (n) [select a note from (n) index at (ch)an]"
					"-n (c) { (b) <(s) } (i)\n"
					"\t[new note with same (i)nfo to (s)ubdiv of (b)eat in (c)hannel]\n"
					"\texample: -n 0 { 2.5 <101 1.5 <01 } bang\n"
					"-n (c) (s) (i) [new note with (i)nfo in (c)hannel at beat (s)tart]\n"
					"-e beat (s) + (sel note) [edit note beat (s)tart]\n" 
					"-e chan (c) + (sel note) [edit note channel]\n"
					"-e info (i) + (sel note) [edit note info]\n"
					"-x + (sel note) [delete note]");
				object_post((t_object *)x,
					"time signature (tsign, ts)\n"
					"-n (b) [new time signature of n (b)eats per bar]\n"
					"-x [delete time signature]");
				object_post((t_object *)x,
					"go to (goto, gt)\n"
					"-n (n) (r) [go to bar (n) (r) times]\n"
					"-x [delete goto]");
				object_post((t_object *)x,
					"tempos (tempo, t)\n"
					"-n (p) [(p)eriod of beat]\n"
					"-n (p) (d) [new beat (p)eriod after (d)elay]\n"
					"-n (p) (d) (v) [change (p)eriod in (v)mseg starting after (d)elay]\n"
					"-n (p) (d) (v) (c) [idem but change with (c)urve]\n"
					"-x [delete tempo]\n");
			}
		}
		else if (atom_gettype(av) == A_LONG || atom_gettype(av) == A_FLOAT) {
			fl_batuta_nextbar(x, (long)atom_getlong(av));
		}
	}
	else if (ac >= 2) {
		total_bars = (long)linklist_getsize(x->l_bars);

		if (atom_gettype(av) == A_SYM && atom_gettype(av + 1) == A_SYM) {
			param = atom_getsym(av);
			accion = atom_getsym(av + 1);

			bar_act = x->jn_bar;

			if (param == gensym("bar") || param == gensym("b")) {
				if (bar_act < 0 || bar_act >= total_bars) { object_warn((t_object *)x, "bar index out of bounds"); return; }
				if (accion == gensym("-n") && ac == 2) {
					err = do_add_bar(x, bar_act + 1);
					if (err) { object_error((t_object *)x, "bar couldn't be added"); return; }
				}
				else if (accion == gensym("-x") && ac == 2) {
					err = do_delete_bar(x, bar_act);
					if (err) { object_error((t_object *)x, "bar couldn't be deleted"); return; }
				}
				else if (accion == gensym("-m") && ac == 3) {
					if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "target bar index must be a number"); return; }
					bar_dest = (long)atom_getlong(av + 2);
					if (bar_dest < 0 || bar_dest > total_bars) { object_warn((t_object *)x, "target bar index out of bounds"); return; }
					err = do_move_bar(x, bar_act, bar_dest);
					if (err) { object_error((t_object *)x, "bar couldn't be moved"); return; }
				}
				else if (accion == gensym("-c") && ac == 3) {
					if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "target bar index must be a number"); return; }
					bar_dest = (long)atom_getlong(av + 2);
					if (bar_dest < 0 || bar_dest > total_bars) { object_warn((t_object *)x, "target bar index out of bounds"); return; }
					err = do_copy_bar(x, bar_act, bar_dest);
					if (err) { object_error((t_object *)x, "bar couldn't be copied"); return; }
				}
				else { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); return; }

				fl_batuta_update_notes(x);
				fl_batuta_update_uibar(x);
				fl_batuta_update_uitempo(x);
			}
			else if (param == gensym("chan") || param == gensym("ch")) {
				if (bar_act >= total_bars || bar_act < 0) { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); return; }
				j_selnota = x->j_selnota;
				
				if (accion == gensym("-e")) {
					if (j_selnota >= 0 && ac == 3) {
						if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "new chan must be a number"); return; }
						chan_orig = x->bars[bar_act]->hdl_nota[j_selnota]->canal;
						chan_final = (long)atom_getlong(av + 2);
					}
					else if (j_selnota < 0 && ac == 4) {
						if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "old chan must be a number"); return; }
						if (atom_gettype(av + 3) != A_FLOAT && atom_gettype(av + 3) != A_LONG) { object_warn((t_object *)x, "new chan must be a number"); return; }
						chan_orig = (long)atom_getlong(av + 2);
						chan_final = (long)atom_getlong(av + 3);
					}
					else { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); return; }
					
					err = do_edit_chan(x, bar_act, chan_orig, chan_final);
					if (err) { object_error((t_object *)x, "chan couldn't be edited"); return; }
					fl_batuta_update_notes_onebar(x, bar_act);
					fl_batuta_update_uibar(x);
				}
				else if (accion == gensym("-x")) {
					if (j_selnota >= 0 && ac == 2) {
						chan_index = x->bars[bar_act]->hdl_nota[j_selnota]->canal;
					}
					else if (j_selnota < 0 && ac == 3) {
						if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "chan must be a number"); return; }
						chan_index = (long)atom_getlong(av + 2);
					}
					else { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); return; }

					err = do_delete_chan(x, bar_act, chan_index);
					if (err) { object_error((t_object *)x, "chan couldn't be deleted"); return; }
					fl_batuta_update_notes_onebar(x, bar_act);
					fl_batuta_update_uibar(x);
				}
				else if (accion == gensym("-c")) {
					if (j_selnota >= 0 && ac == 3) {
						if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "bar must be a number"); return; }
						chan_index = x->bars[bar_act]->hdl_nota[j_selnota]->canal;
						bar_dest = (long)atom_getlong(av + 2);
					}
					else if (j_selnota < 0 && ac == 4) {
						if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "chan must be a number"); return; }
						if (atom_gettype(av + 3) != A_FLOAT && atom_gettype(av + 3) != A_LONG) { object_warn((t_object *)x, "target bar must be a number"); return; }
						chan_index = (long)atom_getlong(av + 2);
						bar_dest = (long)atom_getlong(av + 3);
					}
					else { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); return; }
					
					if (bar_dest < 0 || bar_dest > total_bars){ object_warn((t_object *)x, "pasted bar must be 0 or positive less or equal to the total of bars"); return; }
					err = do_copy_chan(x, chan_index, bar_act, bar_dest);
					if (err) { object_error((t_object *)x, "chan couldn't be copied"); return; }
					fl_batuta_update_notes_onebar(x, bar_dest);
					fl_batuta_update_uibar(x);
				}
				else if (accion == gensym("-h")) {
					if(j_selnota >= 0 && ac == 4){
						if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "fixed range must be a number"); return; }
						if (atom_gettype(av + 3) != A_FLOAT && atom_gettype(av + 3) != A_LONG) { object_warn((t_object *)x, "max random range must be a number"); return; }
						chan_index = x->bars[bar_act]->hdl_nota[j_selnota]->canal;
						rango_fijo = (float)atom_getfloat(av + 2);
						rango_random = (float)atom_getfloat(av + 3);
					}
					else if(j_selnota < 0 && ac == 5){
						if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "chan must be a number"); return; }
						if (atom_gettype(av + 3) != A_FLOAT && atom_gettype(av + 3) != A_LONG) { object_warn((t_object *)x, "fixed range must be a number"); return; }
						if (atom_gettype(av + 4) != A_FLOAT && atom_gettype(av + 4) != A_LONG) { object_warn((t_object *)x, "max random range must be a number"); return; }
						chan_index = (long)atom_getlong(av + 2);
						rango_fijo = (float)atom_getfloat(av + 3);
						rango_random = (float)atom_getfloat(av + 4);
					}
					else { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); return; }

					err = do_human_chan(x, bar_act, chan_index, rango_fijo, rango_random);
					if (err) { object_error((t_object *)x, "chan couldn't be humanized"); return; }
					fl_batuta_update_notes(x);
					fl_batuta_update_uibar(x);
				}
				else if (accion == gensym("-q")) {
					if(j_selnota >= 0 && ac == 3){
						if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "division of a quarter must be a number"); return; }
						chan_index = x->bars[bar_act]->hdl_nota[j_selnota]->canal;
						division = (float)atom_getfloat(av + 2);
					}
					else if(j_selnota < 0 && ac == 4){
						if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "chan must be a number"); return; }
						if (atom_gettype(av + 3) != A_FLOAT && atom_gettype(av + 3) != A_LONG) { object_warn((t_object *)x, "division of a quarter must be a number"); return; }
						chan_index = (long)atom_getlong(av + 2);
						division = (float)atom_getfloat(av + 3);
					}
					else { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); return; }

					if (division < 1.) { object_warn((t_object *)x, "division of quarter must be equal or more than 1."); return; }

					err = do_quantize_chan(x, bar_act, chan_index, division);
					if (err) { object_error((t_object *)x, "chan couldn't be quantized"); return; }
					fl_batuta_update_notes_onebar(x, bar_act);
					fl_batuta_update_uibar(x);
				}
				else { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); }
				x->j_selnota = -1;
				
				if (accion == gensym("-w")) {
					if (ac == 3) {
						if (atom_gettype(av + 2) == A_SYM) {
							if (atom_getsym(av + 2) == gensym("all")) { x->jn_show_all_notes = 1; }
						}
						else if (atom_gettype(av + 2) == A_FLOAT || atom_gettype(av + 2) == A_LONG) {
							chan_index = (long)atom_getlong(av + 2);
							x->jn_chan_min = x->jn_chan_max = chan_index;
							x->jn_show_all_notes = 0;
						}
					}
					if (ac == 4) {
						if ((atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) ||
							(atom_gettype(av + 3) != A_FLOAT && atom_gettype(av + 3) != A_LONG)) {
							object_warn((t_object *)x, "range values must be numbers"); return;
						}
						chan_orig = (long)atom_getlong(av + 2);
						chan_final = (long)atom_getlong(av + 3);
						x->jn_chan_min = MIN(chan_orig, chan_final);
						x->jn_chan_min = MAX(chan_orig, chan_final);
						x->jn_show_all_notes = 0;
					}
					jbox_invalidate_layer((t_object *)x, NULL, gensym("bars_layer"));
					jbox_invalidate_layer((t_object *)x, NULL, gensym("notes_layer"));
					jbox_redraw((t_jbox *)x);
				}
			}
			else if (param == gensym("note") || param == gensym("n")) {
				j_selnota = x->j_selnota;

				if (bar_act >= total_bars || bar_act < 0) { object_warn((t_object *)x, "bar index out of bounds"); return; }

				if (accion == gensym("-s") && ac == 4) {
					if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "channel must be a number"); return; }
					if (atom_gettype(av + 3) != A_FLOAT && atom_gettype(av + 3) != A_LONG) { object_warn((t_object *)x, "note index must be a number"); return; }
					chan_index = (long)atom_getlong(av + 2);
					note_index = (long)atom_getlong(av + 3);
					if (note_index < 0) { object_warn((t_object *)x, "index must be 0 or positive"); return; }

					err = fl_batuta_is_note_index(x, &x->j_selnota, bar_act, chan_index, note_index);
					if (err) { object_error((t_object *)x, "note couldn't be selected"); return; }
					
					jbox_invalidate_layer((t_object *)x, NULL, gensym("bars_layer"));
					jbox_invalidate_layer((t_object *)x, NULL, gensym("info_layer"));
					jbox_invalidate_layer((t_object *)x, NULL, gensym("notes_layer"));
					jbox_redraw((t_jbox *)x);
				}
				else if (accion == gensym("-n") && ac > 3) {
					if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "channel must be a number"); return; }
					if (atom_gettype(av + 3) != A_SYM) { object_warn((t_object *)x, "4th arg must be a '{' character"); return; }
					chan_index = (long)atom_getlong(av + 2);
					subcat = atom_getsym(av + 3);

					if (subcat == gensym("{")) { //'{' index is odd, '}' index is even
						//buscar final repetidor
						index_repf = -1;
						int ii = 3;
						while (++ii < ac) {
							if (atom_gettype(av + ii) == A_SYM) {
								if (atom_getsym(av + ii) == gensym("}")) { index_repf = ii; break; }
							}
						}
						if (index_repf == -1) { object_warn((t_object *)x, "'}' character not found"); return; }
						if (index_repf % 2 == 1) { object_warn((t_object *)x, "values between '{ }' must be even: beat, subdiv, beat, subdiv..."); return; }

						//contar subdiv
						ii = 3;
						beat_acum = beat = 0.;
						while (++ii < index_repf) {
							if (ii % 2 == 0) { //even: beat
								if (atom_gettype(av + ii) != A_FLOAT && atom_gettype(av + ii) != A_LONG) { object_warn((t_object *)x, "beat must be a number"); return; }
								beat_acum += beat;
								beat = (float)atom_getfloat(av + ii);
							}
							else { //odd: list
								if (atom_gettype(av + ii) != A_SYM) { object_warn((t_object *)x, "list must be a symbol: a '<' char and a list of 0/1s"); return; }
								str_lista = atom_getsym(av + ii)->s_name;
								if (str_lista[0] != '<') { object_warn((t_object *)x, "'<' character not found"); return; }

								index_string = 1;
								subdiv = 0;
								while (str_lista[index_string++] != '\0') {
									subdiv++;
								}

								index_string = 0;
								while (++index_string <= subdiv) {
									if (str_lista[index_string] == '1' && ac > index_repf + 1) {
										inicio = beat_acum + beat * (float)(index_string - 1) / (float)subdiv;
										err = do_add_note(x, bar_act, inicio, chan_index, ac - (index_repf + 1), av + index_repf + 1);
										if(err){ object_error((t_object *)x, "note couldn't be added"); return; }
									}
								}
							}
						}
					}
					else {
						if (atom_gettype(av + 3) != A_FLOAT && atom_gettype(av + 3) != A_LONG) { object_warn((t_object *)x, "beat start must be a number"); return; }
						inicio = (float)atom_getfloat(av + 3);
						if (inicio >= 0. && ac > 4) {
							err = do_add_note(x, bar_act, inicio, chan_index, ac - 4, av + 4);
							if (err) { object_error((t_object *)x, "note couldn't be added"); return; }
						}
					}
					fl_batuta_update_notes_onebar(x, bar_act);
					fl_batuta_update_uibar(x);
				}
				else if (accion == gensym("-e") && ac > 3) {
					if (j_selnota < 0) { object_warn((t_object *)x, "select a note first"); return; }
					subcat = atom_getsym(av + 2);

					if ((subcat == gensym("beat") || subcat == gensym("b")) && ac == 4) {
						inicio = (float)atom_getfloat(av + 3);
						err = do_edit_note_start(x, bar_act, j_selnota, inicio);
					}
					else if ((subcat == gensym("chan") || subcat == gensym("ch")) && ac == 4) {
						chan_index = (long)atom_getlong(av + 3);
						err = do_edit_note_chan(x, bar_act, j_selnota, chan_index);
					}
					else if ((subcat == gensym("info") || subcat == gensym("i")) && ac >= 4) {
						err = do_edit_note_info(x, bar_act, j_selnota, ac - 3, av + 3);
					}
					if (err) { object_error((t_object *)x, "note couldn't be edited"); return; }
					fl_batuta_update_notes_onebar(x, bar_act);
					fl_batuta_update_uibar(x);
					x->j_selnota = -1;
				}
				else if (accion == gensym("-x") && ac == 2) {
					if (j_selnota < 0) { return; }
					err = do_delete_note(x, bar_act, j_selnota);
					if (err) { object_error((t_object *)x, "note couldn't be deleted"); return; }
					fl_batuta_update_notes_onebar(x, bar_act);
					fl_batuta_update_uibar(x);
					x->j_selnota = -1;
				}
				else { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); }
			}
			else if (param == gensym("tsign") || param == gensym("ts")) {
				bar_act = x->jn_bar;
				if (bar_act >= total_bars || bar_act < 0) { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); return; }

				if (accion == gensym("-n") && ac == 3) {
					if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "beats must be a number"); return; }
					beats = (float)atom_getfloat(av + 2);

					if (beats < 1.) { object_warn((t_object *)x, "numerator must be more than 1."); return; }

					err = do_add_signature(x, bar_act, beats);
					if (err) { object_error((t_object *)x, "time signature couldn't be added"); return; }
					fl_batuta_update_signatures(x);
					fl_batuta_update_uibar(x);
				}
				else if (accion == gensym("-x") && ac == 2) {
					err = do_delete_signature(x, bar_act);
					if (err) { object_error((t_object *)x, "time signature couldn't be deleted"); return; }
					fl_batuta_update_signatures(x);
					fl_batuta_update_uibar(x);
				}
				else { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); }
			}
			else if (param == gensym("goto") || param == gensym("gt")) {
				bar_act = x->jn_bar;
				if (bar_act >= total_bars || bar_act < 0) { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); return; }

				if (accion == gensym("-n") && ac == 4) {
					if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "target bar index must be a number"); return; }
					if (atom_gettype(av + 3) != A_FLOAT && atom_gettype(av + 3) != A_LONG) { object_warn((t_object *)x, "repetitions must be a number"); return; }
					bar_dest = (long)atom_getlong(av + 2);
					rep = (long)atom_getlong(av + 3);

					if (bar_dest < 0 || bar_dest >= total_bars) { object_warn((t_object *)x, "target bar index must be less than the total of bars"); return; }
					if (rep <= 0) { object_warn((t_object *)x, "repetitions must be more a positive"); return; }

					err = do_add_goto(x, bar_act, bar_dest, rep);
					if (err) { object_error((t_object *)x, "time signature couldn't be deleted"); return; }
					fl_batuta_update_gotos(x);
					fl_batuta_update_uigoto(x);
				}
				else if (accion == gensym("-x") && ac == 2) {
					err = do_delete_goto(x, bar_act);
					if (err) { object_error((t_object *)x, "goto couldn't be deleted"); return; }
					fl_batuta_update_gotos(x);
					fl_batuta_update_uigoto(x);
				}
				else { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); }
			}
			else if (param == gensym("tempo") || param == gensym("t")) {
				bar_act = x->jn_bar;
				if (bar_act >= total_bars || bar_act < 0) { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); return; }

				if (accion == gensym("-n")) {
					if (ac > 2) {//mstempo
						if (atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) { object_warn((t_object *)x, "period must be a number"); return; }
						tipo = 0;
						mstempo = (float)atom_getfloat(av + 2);
						inicio = 0.;
						variacion = 0.;
						curva = 0.;
					}
					if (ac > 3) {//inicio, mstempo
						if (atom_gettype(av + 3) != A_FLOAT && atom_gettype(av + 3) != A_LONG) { object_warn((t_object *)x, "start must be a number"); return; }
						tipo = 0;
						inicio = (float)atom_getfloat(av + 3);
					}
					else if (ac == 5) {//inicio, mstempo, variacion
						if (atom_gettype(av + 4) != A_FLOAT && atom_gettype(av + 4) != A_LONG) { object_warn((t_object *)x, "variation must be a number"); return; }
						tipo = 1;
						variacion = (float)atom_getfloat(av + 4);
					}
					else if (ac == 6) {//inicio, mstempo,variacion,curva
						if (atom_gettype(av + 5) != A_FLOAT && atom_gettype(av + 5) != A_LONG) { object_warn((t_object *)x, "curve must be a number"); return; }
						tipo = 2;
						curva = (float)atom_getfloat(av + 5);
						curva = MIN(MAX(curva, CURVE_MIN), CURVE_MAX);
					}

					if (inicio < 0.) { object_warn((t_object *)x, "start must be 0 or positive"); return; }
					if (mstempo < TEMPO_MIN) { object_warn((t_object *)x, "period must be more than %f", TEMPO_MIN); return; }
					if (variacion < 0.) { object_warn((t_object *)x, "delta duration must be 0 or positive"); return; }
						
					err = do_add_tempo(x, tipo, bar_act, inicio, mstempo, variacion, curva);
					if (err) { object_error((t_object *)x, "tempo couldn't be added"); return; }
					fl_batuta_update_tempos(x);
					fl_batuta_update_uitempo(x);
				}
				else if (accion == gensym("-x") && ac == 2) {
					err = do_delete_tempo(x, bar_act);
					if (err) { object_error((t_object *)x, "tempo couldn't be deleted"); return; }
					fl_batuta_update_tempos(x);
					fl_batuta_update_uitempo(x);
				}
				else { object_warn((t_object *)x, "Something is wrong. Press '-i' to see command list"); }
			}
		}
	}
}