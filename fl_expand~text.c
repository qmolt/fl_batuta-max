#include "fl_batuta~.h"

long fl_batuta_key(t_fl_batuta *x, t_object *patcherview, long keycode, long modifiers, long textcharacter)
{
	char buff[256];
	buff[0] = (char)textcharacter;  // we know this is just a simple char
	buff[1] = 0;
	object_method(patcherview, gensym("insertboxtext"), x, buff);
	jbox_redraw((t_jbox *)x);

	return 1;
}

long fl_batuta_keyfilter(t_fl_batuta *x, t_object *patcherview, long *keycode, long *modifiers, long *textcharacter)
{
	t_atom arv;
	long rv = 1;
	long k = *keycode;

	if (k == JKEY_TAB || k == JKEY_ENTER || k == JKEY_ESC) {//|| k == JKEY_RETURN
		object_method_typed(patcherview, gensym("endeditbox"), 0, NULL, &arv);
		rv = 0;		// don't pass those keys to uitextfield
	}
	return rv;
}

void fl_batuta_enter(t_fl_batuta *x)	// enter is triggerd at "endeditbox time"
{
	long ac = 0;
	t_atom *av = x->parsed_atoms;
	t_symbol *param, *accion, *subcat;
	long size = 0;
	char *text = NULL;
	t_object *textfield = jbox_get_textfield((t_object *)x);

	long total_bars, bar_act, bar_dest;
	long chan_orig, chan_final, chan_index;
	long denom, rep;
	float numer;
	long j_selnota;
	short tipo;
	float mstempo, variacion, inicio, curva;
	t_max_err err;
	long index_repf, index_string, subdiv;
	float beat_acum, beat;
	char *str_lista;
	float rango_fijo, rango_random, division;

	object_method(textfield, gensym("gettextptr"), &text, &size);
	if (size) {
		my_setparse(&ac, &av, text);

		if (ac == 1) {
			if (atom_gettype(av) == A_SYM) {
				accion = atom_getsym(av);
				if (accion == ps_acc_info) {
					object_post((t_object *)x,
						"compas\n"
						"-n [crea compas]\n"
						"-x [borra compas]\n"
						"-m (n) [mueve compas a (n)]\n"
						"-c (n) [copia compas a (n)]");
					object_post((t_object *)x,
						"canal\n"
						"-e (n) [edita notas de canal a (n)]\n"
						"-x [borra notas del canal]\n"
						"-c (n) [copia canal a compas (n)]\n"
						"-h (f) (r) [desplaza +/- %%(f)ijo y %%(r)andom]\n"
						"-q (d) [cuantiza a (d)ivision de unidad]");
					object_post((t_object *)x,
						"nota\n"
						"-n { (b) <(s) } (c) (i)\n"
						"\t[crea (i)nfo por (s)ubdiv de (b)eat en (c)anal]\n"
						"-n (i) (c) (i) [crea (i)nfo en (c)anal con (i)nicio]\n"
						"-e inicio (i) / -e canal (c) / -e info (i) [edita]\n"
						"-x [borra nota]");
					object_post((t_object *)x,
						"cifra\n"
						"-n (n) (d) [cifra: (n)umerador (d)enominador]\n"
						"-x [borra cifra]");
					object_post((t_object *)x,
						"goton\n"
						"-n (n) (r) [ir a compas (n) (r) veces]\n"
						"-x [borra goto]");
					object_post((t_object *)x,
						"tempo\n"
						"-n (d) (b) [cambiar a tempo(b) con (d)elay]\n"
						"-n (d) (b) (v) [idem durante (v)mseg]\n"
						"-n (d) (b) (v) (c) [idem con (v)ariacion (c)urvada]\n"
						"-x [borra tempo]\n");
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

				if (param == ps_cat_compas) {
					if (accion == ps_acc_nuevo && ac == 2) {
						if (bar_act <= total_bars && bar_act >= 0) {
							do_add_bar(x, bar_act);
							fl_batuta_actualizar_notas(x);
						}
						else { object_post((t_object *)x, "???"); }
					}
					else if (accion == ps_acc_borrar && ac == 2) {
						if (bar_act < total_bars && bar_act >= 0) {
							do_delete_bar(x, bar_act);
							fl_batuta_actualizar_notas(x);
						}
						else { object_post((t_object *)x, "???"); }
					}
					else if (accion == ps_acc_mover && ac == 3) {
						bar_dest = (long)atom_getlong(av + 2);
						if (bar_act < total_bars && bar_act >= 0) {
							do_move_bar(x, bar_act, bar_dest);
							fl_batuta_actualizar_notas(x);
						}
						else { object_post((t_object *)x, "???"); }
					}
					else if (accion == ps_acc_copiar && ac == 3) {
						bar_dest = (long)atom_getlong(av + 2);
						if (bar_act < total_bars && bar_act >= 0) {
							do_copy_bar(x, bar_act, bar_dest);
							fl_batuta_actualizar_notas(x);
						}
						else { object_post((t_object *)x, "???"); }
					}
					else { object_post((t_object *)x, "???"); }
				}
				else if (param == ps_cat_canal) {
					j_selnota = x->j_selnota;

					if (bar_act >= total_bars || bar_act < 0) { object_post((t_object *)x, "???"); return; }

					if (j_selnota >= 0) {
						if (accion == ps_acc_editar && ac == 3) {
							chan_orig = x->compases[bar_act]->hdl_nota[j_selnota]->canal;
							chan_final = (long)atom_getlong(av + 2);

							do_edit_chan(x, bar_act, chan_orig, chan_final);
							fl_batuta_actualizar_notas(x);
						}
						else if (accion == ps_acc_borrar && ac == 2) {
							chan_index = x->compases[bar_act]->hdl_nota[j_selnota]->canal;

							do_delete_chan(x, bar_act, chan_index);
							fl_batuta_actualizar_notas(x);
						}
						else if (accion == ps_acc_copiar && ac == 3) {
							chan_index = x->compases[bar_act]->hdl_nota[j_selnota]->canal;
							bar_dest = (long)atom_getlong(av + 2);

							do_copy_chan(x, chan_index, bar_act, bar_dest);
							fl_batuta_actualizar_notas(x);
						}
						else if (accion == ps_acc_human && ac == 4) {
							chan_index = x->compases[bar_act]->hdl_nota[j_selnota]->canal;
							rango_fijo = (float)atom_getfloat(av + 2);
							rango_random = (float)atom_getfloat(av + 3);

							do_human_chan(x, bar_act, chan_index, rango_fijo, rango_random);
						}
						else if (accion == ps_acc_quantize && ac == 3) {
							chan_index = x->compases[bar_act]->hdl_nota[j_selnota]->canal;
							division = (float)atom_getfloat(av + 2);

							if (division > 0.) {
								do_quantize_chan(x, bar_act, chan_index, division);
							}
						}
						else { object_post((t_object *)x, "???"); }
					}
					x->j_selnota = -1;
				}
				else if (param == ps_cat_nota) {
					j_selnota = x->j_selnota;

					if (bar_act >= total_bars || bar_act < 0) { object_post((t_object *)x, "???"); return; }

					if (accion == ps_acc_nuevo && ac > 3) {

						if (atom_gettype(av + 2) == A_SYM) { subcat = atom_getsym(av + 2); }
						else { subcat = gensym("no"); }

						if (subcat == ps_acc_repi) {
							//buscar final repetidor
							index_repf = -1;
							int ii = 2;
							while (++ii < ac) {
								if (atom_gettype(av + ii) == A_SYM) {
									if (atom_getsym(av + ii) == ps_acc_repf) { index_repf = ii; break; }
								}
							}
							if (index_repf == -1) { return; }
							if (index_repf % 2 == 0) { return; }

							//contar subdiv
							ii = 2;
							beat_acum = beat = 0.;
							while (++ii < index_repf) {
								if (ii % 2 == 1) { //impar: beat
									if (atom_gettype(av + ii) != A_FLOAT && atom_gettype(av + ii) != A_LONG) { return; }
									beat_acum += beat;
									beat = (float)atom_getfloat(av + ii);
								}
								else { //par: lista
									if (atom_gettype(av + ii) != A_SYM) { continue; }
									str_lista = atom_getsym(av + ii)->s_name;
									if (str_lista[0] != '<') { continue; }

									index_string = 0;
									subdiv = 0;
									while (str_lista[++index_string] != '\0') {
										subdiv++;
									}

									index_string = 0;
									while (++index_string <= subdiv) {
										if (str_lista[index_string] == '1' && ac > index_repf + 1) {
											inicio = beat_acum + beat * (float)(index_string - 1) / (float)subdiv;
											chan_index = (long)atom_getlong(av + index_repf + 1);
											err = do_add_note(x, bar_act, inicio, chan_index, ac - (index_repf + 2), av + index_repf + 2);
										}
									}
								}
							}

							//actualizar
							fl_batuta_actualizar_notas(x);
						}
						else {
							inicio = (float)atom_getfloat(av + 2);
							chan_index = (long)atom_getlong(av + 3);
							if (inicio >= 0. && ac > 4) {
								err = do_add_note(x, bar_act, inicio, chan_index, ac - 4, av + 4);
								if (!err) { fl_batuta_actualizar_notas(x); }
							}
						}
					}
					else if (accion == ps_acc_editar && ac > 3) {
						if (j_selnota < 0) { return; }
						subcat = atom_getsym(av + 2);

						if (subcat == ps_subc_nota_inicio && ac == 4) {
							inicio = (float)atom_getfloat(av + 3);
							err = do_edit_note_inicio(x, bar_act, j_selnota, inicio);
							if (!err) { fl_batuta_actualizar_notas(x); }
						}
						else if (subcat == ps_subc_nota_canal && ac == 4) {
							chan_index = (long)atom_getlong(av + 3);
							err = do_edit_note_canal(x, bar_act, j_selnota, chan_index);
							if (!err) { fl_batuta_actualizar_notas(x); }
						}
						else if (subcat == ps_subc_nota_info && ac >= 4) {
							err = do_edit_note_info(x, bar_act, j_selnota, ac - 3, av + 3);
							if (!err) { fl_batuta_actualizar_notas(x); }
						}
					}
					else if (accion == ps_acc_borrar && ac == 2) {
						if (j_selnota < 0) { return; }
						err = do_delete_note(x, bar_act, j_selnota);
						if (!err) { fl_batuta_actualizar_notas(x); }
					}
					else { object_post((t_object *)x, "???"); }

					x->j_selnota = -1;
				}
				else if (param == ps_cat_cifra) {
					bar_act = x->jn_bar;
					if (bar_act >= total_bars || bar_act < 0) { object_post((t_object *)x, "???"); return; }

					if (accion == ps_acc_nuevo && ac == 4) {
						bar_act = x->jn_bar;
						numer = (float)atom_getfloat(av + 2);
						denom = (long)atom_getlong(av + 3);

						if (numer > 0 && denom > 0) {
							do_add_cifra(x, bar_act, numer, denom);
							fl_batuta_actualizar_cifras(x);
						}
						else { object_post((t_object *)x, "???"); }
					}
					else if (accion == ps_acc_borrar && ac == 2) {
						do_delete_cifra(x, bar_act);
						fl_batuta_actualizar_cifras(x);
					}
					else { object_post((t_object *)x, "???"); }
				}
				else if (param == ps_cat_goton) {
					bar_act = x->jn_bar;
					if (bar_act >= total_bars || bar_act < 0) { object_post((t_object *)x, "???"); return; }

					if (accion == ps_acc_nuevo && ac == 4) {
						bar_act = x->jn_bar;
						bar_dest = (long)atom_getlong(av + 2);
						rep = (long)atom_getlong(av + 3);
						if (bar_dest < total_bars && bar_dest >= 0 && rep > 0) {
							do_add_goto(x, bar_act, bar_dest, rep);
							fl_batuta_actualizar_gotos(x);
						}
						else { object_post((t_object *)x, "???"); }
					}
					else if (accion == ps_acc_borrar && ac == 2) {
						do_delete_goto(x, bar_act);
						fl_batuta_actualizar_gotos(x);
					}
					else { object_post((t_object *)x, "???"); }
				}
				else if (param == ps_cat_tempo) {
					bar_act = x->jn_bar;
					if (bar_act >= total_bars || bar_act < 0) { object_post((t_object *)x, "???"); return; }

					if (accion == ps_acc_nuevo) {
						if (ac == 4) {//inicio, mstempo
							tipo = 0;
							inicio = (float)atom_getfloat(av + 2);
							mstempo = (float)atom_getfloat(av + 3);
							variacion = 0.0;
							curva = 0.5;
							if (inicio >= 0 && mstempo > 0) {
								do_add_tempo(x, tipo, bar_act, inicio, mstempo, variacion, curva);
								fl_batuta_actualizar_tempos(x);
							}
							else { object_post((t_object *)x, "???"); }
						}
						else if (ac == 5) {//inicio, mstempo, variacion
							tipo = 1;
							inicio = (float)atom_getfloat(av + 2);
							mstempo = (float)atom_getfloat(av + 3);
							variacion = (float)atom_getfloat(av + 4);
							curva = 0.5;
							if (inicio >= 0 && mstempo > 0 && variacion >= 0) {
								do_add_tempo(x, tipo, bar_act, inicio, mstempo, variacion, curva);
								fl_batuta_actualizar_tempos(x);
							}
							else { object_post((t_object *)x, "???"); }
						}
						else if (ac == 6) {//inicio, mstempo,variacion,curva
							tipo = 2;
							inicio = (float)atom_getfloat(av + 2);
							mstempo = (float)atom_getfloat(av + 3);
							variacion = (float)atom_getfloat(av + 4);
							curva = (float)atom_getfloat(av + 5);
							if (inicio >= 0 && mstempo > 0 && variacion >= 0 && curva >= CURVE_MIN && curva <= CURVE_MAX) {
								do_add_tempo(x, tipo, bar_act, inicio, mstempo, variacion, curva);
								fl_batuta_actualizar_tempos(x);
							}
							else { object_post((t_object *)x, "???"); }
						}
					}
					else if (accion == ps_acc_borrar) {
						do_delete_tempo(x, bar_act);
						fl_batuta_actualizar_tempos(x);
					}
					else { object_post((t_object *)x, "???"); }
				}
			}
		}
		//post("This is the new text: %s", text);
	}
}