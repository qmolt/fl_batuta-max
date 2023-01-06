#include "flbatuta~.h"

/*almacenar---------------------------------------------------------------------------------------------*/
void fl_batuta_read(t_fl_batuta *x, t_symbol *s)
{
	if (x->isplaying) { object_warn((t_object *)x, "can't open a file while playing"); return; }
	if (x->isediting) { object_warn((t_object *)x, "can't open a file while editing"); return; }
	if (x->isloading) { object_warn((t_object *)x, "already reading/writing a file"); return; }

	defer(x, (method)fl_batuta_doread, s, 0, NULL);
}

void fl_batuta_doread(t_fl_batuta *x, t_symbol *s)
{
	t_fourcc filetype = 'JSON', outtype = 'JSON';
	short numtypes = 1;
	char filename[MAX_PATH_CHARS];
	short path;
	if (s == gensym("")) {      // if no argument supplied, ask for file
		open_promptset("choose a JSON file");
		if (open_dialog(filename, &path, &outtype, &filetype, 1)) {      // non-zero: user cancelled
			return;
		}
	}
	else {
		strcpy(filename, s->s_name);    // must copy symbol before calling locatefile_extended
		if (locatefile_extended(filename, &path, &outtype, &filetype, 1)) { // non-zero: not found
			object_error((t_object *)x, "%s: not found", s->s_name);
			return;
		}
	}
	object_post((t_object *)x, "loading: %s ", filename);
	fl_batuta_openfile(x, filename, path);
}

void fl_batuta_openfile(t_fl_batuta *x, char *filename, short path)
{
	t_filehandle fh;
	char *buffer;
	t_ptr_size zusize;
	long size;
	t_max_err err = MAX_ERR_NONE;

	struct json_object *jobj, *tmp, *tmpidx, *tmpval, *tmpcurve;
	json_type val_type;

	fl_bar *pbar;
	long total_notas;
	fl_note *pnota;

	long total_bars = 0;
	long total_tempos = 0;
	long total_tsigns = 0;
	long total_gotos = 0;
	long total_notes = 0;

	long addcounter;

	long t_bar;
	float t_curve;
	float t_msbeat;
	float t_msdurvar;
	float t_msstart;

	long ts_bar;
	float ts_beats;

	long g_bar;
	long g_tobar;
	long g_rep;
	
	long n_bar;
	float n_start;
	long n_chan;
	long n_ac = 128;
	t_atom *n_ap = (t_atom *)sysmem_newptr(128 * sizeof(t_atom *));
	char *n_message;

	if (path_opensysfile(filename, path, &fh, READ_PERM)) {
		object_error((t_object *)x, "error opening %s", filename);
		return;
	}

	x->isloading = 1;

	//clear lists
	total_bars = (long)linklist_getsize(x->l_bars);
	for (long i = 0; i < total_bars; i++) {
		pbar = linklist_getindex(x->l_bars, i);
		total_notas = (long)linklist_getsize(pbar->notas);
		for (long k = 0; k < total_notas; k++) {
			pnota = linklist_getindex(pbar->notas, k);
			sysmem_freeptr(pnota->pnota);
		}
		object_free(pbar->notas);
	}
	linklist_clear(x->l_bars);
	linklist_clear(x->l_tempos);
	linklist_clear(x->l_tsigns);
	linklist_clear(x->l_gotos);

	total_bars = 0;
	total_notes = 0;

	//--------------------------------------------------------
	//parse JSON
	sysfile_geteof(fh, &zusize);
	size = zusize;
	buffer = (char *)sysmem_newptr(size);
	if (!buffer) { object_error((t_object *)x, "out of memory to read file"); return; }
	sysfile_read(fh, &zusize, buffer);
	sysfile_close(fh);

	jobj = json_tokener_parse(buffer);
	if (!jobj) {
		object_error((t_object *)x, "couldn't parse JSON file");
		sysmem_freeptr(buffer);
		return;
	}

	//-----------------------------------------------------------------
	//bars
	if (json_object_object_get_ex(jobj, "bars", &tmp)) {
		val_type = json_object_get_type(tmp);
		if (val_type == json_type_int) {
			total_bars = json_object_get_int64(tmp);
		}
	}
	if (total_bars > 0) {
		addcounter = 0;
		for (long i = 0; i < total_bars; i++) {
			err = do_add_bar(x, 0);
			if (err) { object_error((t_object *)x, "read error: bar couldn't be added"); }
			else { addcounter++; }
		}
	}
	else {
		err = do_add_bar(x, 0);
		if (err) { object_error((t_object *)x, "read error: bar couldn't be added"); }
		else { addcounter = 1; }
	}
	object_post((t_object *)x, "bars added: %d of %d", addcounter, total_bars);

	//-----------------------------------------------------------------
	//tempo
	if (json_object_object_get_ex(jobj, "tempo", &tmp)) {
		val_type = json_object_get_type(tmp);
		if (val_type == json_type_array) {
			total_tempos = json_object_array_length(tmp);
			addcounter = 0;
			
			if (total_tempos > 0) {
				for (long i = 0; i < total_tempos; i++) {
					tmpidx = json_object_array_get_idx(tmp, i);

					if (json_object_object_get_ex(tmpidx, "bar", &tmpval)) {
						val_type = json_object_get_type(tmpval);
						if (val_type != json_type_int) { continue; }
						t_bar = json_object_get_int64(tmpval);
					}
					else { continue; }

					if (json_object_object_get_ex(tmpidx, "msbeat", &tmpval)) {
						val_type = json_object_get_type(tmpval);
						if (val_type != json_type_double) { continue; }
						t_msbeat = json_object_get_double(tmpval);
					}
					else { continue; }

					if (json_object_object_get_ex(tmpidx, "curve", &tmpval)) {
						val_type = json_object_get_type(tmpval);
						if (val_type == json_type_array) {
							if (json_object_array_length(tmpval) != 3) { continue; }
							else {
								tmpcurve = json_object_array_get_idx(tmpval, 0);
								val_type = json_object_get_type(tmpcurve);
								if (val_type != json_type_double) { continue; }
								t_msstart = json_object_get_double(tmpcurve);

								tmpcurve = json_object_array_get_idx(tmpval, 1);
								val_type = json_object_get_type(tmpcurve);
								if (val_type != json_type_double) { continue; }
								t_msdurvar = json_object_get_double(tmpcurve);

								tmpcurve = json_object_array_get_idx(tmpval, 2);
								val_type = json_object_get_type(tmpcurve);
								if (val_type != json_type_double) { continue; }
								t_curve = json_object_get_double(tmpcurve);
							}
						}
					}
					else { continue; }
			
					err = do_add_tempo(x, 1, t_bar, t_msstart, t_msbeat, t_msdurvar, t_curve);
					if (err) { object_error((t_object *)x, "read error: tempo couldn't be added"); }
					else { addcounter++; }
				}
			}
			else {
				err = do_add_tempo(x, 1, 0, 0., 500., 0., 0.);
				if (err) { object_error((t_object *)x, "read error: tempo couldn't be added"); }
			}
		}
	}
	object_post((t_object *)x, "tempos added: %d of %d", addcounter, total_tempos);

	//-----------------------------------------------------------------
	//tsign
	if (json_object_object_get_ex(jobj, "tsign", &tmp)) {
		val_type = json_object_get_type(tmp);
		if (val_type == json_type_array) {
			total_tsigns = json_object_array_length(tmp);
			addcounter = 0;

			if (total_tsigns > 0) {
				for (long i = 0; i < total_tsigns; i++) {
					tmpidx = json_object_array_get_idx(tmp, i);

					if (json_object_object_get_ex(tmpidx, "bar", &tmpval)) {
						val_type = json_object_get_type(tmpval);
						if (val_type != json_type_int) { continue; }
						ts_bar = json_object_get_int64(tmpval);
					}
					else { continue; }

					if (json_object_object_get_ex(tmpidx, "beats", &tmpval)) {
						val_type = json_object_get_type(tmpval);
						if (val_type != json_type_double) { continue; }
						ts_beats = json_object_get_double(tmpval);
					}
					else { continue; }

					err = do_add_signature(x, ts_bar, ts_beats);
					if (err) { object_error((t_object *)x, "read error: tsign couldn't be added"); }
					else { addcounter++; }
				}
			}
			else {
				err = do_add_signature(x, 0, 4.);
				if (err) { object_error((t_object *)x, "read error: tsign couldn't be added"); }
			}
		}
	}
	object_post((t_object *)x, "tsigns added: %d of %d", addcounter, total_tsigns);

	//-----------------------------------------------------------------
	//goto
	if (json_object_object_get_ex(jobj, "goto", &tmp)) {
		val_type = json_object_get_type(tmp);
		if (val_type == json_type_array) {
			total_gotos = json_object_array_length(tmp);
			addcounter = 0;

			if (total_gotos > 0) {
				for (long i = 0; i < total_gotos; i++) {
					tmpidx = json_object_array_get_idx(tmp, i);

					if (json_object_object_get_ex(tmpidx, "bar", &tmpval)) {
						val_type = json_object_get_type(tmpval);
						if (val_type != json_type_int) { continue; }
						g_bar = json_object_get_int64(tmpval);
					}
					else { continue; }

					if (json_object_object_get_ex(tmpidx, "tobar", &tmpval)) {
						val_type = json_object_get_type(tmpval);
						if (val_type != json_type_int) { continue; }
						g_tobar = json_object_get_int64(tmpval);
					}
					else { continue; }

					if (json_object_object_get_ex(tmpidx, "rep", &tmpval)) {
						val_type = json_object_get_type(tmpval);
						if (val_type != json_type_int) { continue; }
						g_rep = json_object_get_int64(tmpval);
					}
					else { continue; }

					err = do_add_goto(x, g_bar, g_tobar, g_rep);
					if (err) { object_error((t_object *)x, "read error: goto couldn't be added"); }
					else { addcounter++; }
				}
			}
		}
	}
	object_post((t_object *)x, "gotos added: %d of %d", addcounter, total_gotos);
	
	//-----------------------------------------------------------------
	//note
	if (json_object_object_get_ex(jobj, "note", &tmp)) {
		val_type = json_object_get_type(tmp);
		if (val_type == json_type_array) {
			total_notes = json_object_array_length(tmp);
			addcounter = 0;

			if (total_notes > 0) {
				for (long i = 0; i < total_notes; i++) {
					tmpidx = json_object_array_get_idx(tmp, i);

					if (json_object_object_get_ex(tmpidx, "bar", &tmpval)) {
						val_type = json_object_get_type(tmpval);
						if (val_type != json_type_int) { continue; }
						n_bar = json_object_get_int64(tmpval);
					}
					else { continue; }

					if (json_object_object_get_ex(tmpidx, "start", &tmpval)) {
						val_type = json_object_get_type(tmpval);
						if (val_type != json_type_double) { continue; }
						n_start = json_object_get_double(tmpval);
					}
					else { continue; }

					if (json_object_object_get_ex(tmpidx, "chan", &tmpval)) {
						val_type = json_object_get_type(tmpval);
						if (val_type != json_type_int) { continue; }
						n_chan = json_object_get_int64(tmpval);
					}
					else { continue; }

					if (json_object_object_get_ex(tmpidx, "message", &tmpval)) {
						val_type = json_object_get_type(tmpval);
						if (val_type != json_type_string) { continue; }
						n_message = json_object_get_string(tmpval);

						err = atom_setparse(&n_ac, &n_ap, n_message);
						if (!err) {
							if (!n_ap) { n_ap = (t_atom *)sysmem_newptr(n_ac * sizeof(t_atom)); }
							else { n_ap = (t_atom *)sysmem_resizeptr(n_ap, n_ac * sizeof(t_atom)); }
							
							err = do_add_note(x, n_bar, n_start, n_chan, n_ac, n_ap);
							if (err) { object_error((t_object *)x, "read error: note couldn't be added"); }
							else { addcounter++; }
						}
					}
					else { continue; }
				}
			}
		}
	}
	if (n_ap) { sysmem_freeptr(n_ap); }
	
	object_post((t_object *)x, "notes added: %d of %d", addcounter, total_notes);
	
	//-----------------------------------------------------------------
	sysmem_freeptr(buffer);
	sysmem_freeptr(n_ap);

	fl_batuta_update_tempos(x);
	fl_batuta_update_signatures(x);
	fl_batuta_update_gotos(x);
	fl_batuta_update_notes(x);
	fl_batuta_update_uibar(x);
	fl_batuta_update_uitempo(x);
	fl_batuta_update_uigoto(x);

	object_post((t_object *)x, "file opened with no errors");
	x->isloading = 0;
}

void fl_batuta_write(t_fl_batuta *x, t_symbol *s)
{
	if (x->isplaying) { object_warn((t_object *)x, "can't write a file while playing"); return; }
	if (x->isediting) { object_warn((t_object *)x, "can't write a file while editing"); return; }
	if (x->isloading) { object_warn((t_object *)x, "already reading/writing a file"); return; }

	defer(x, (method)fl_batuta_dowrite, s, 0, NULL);
}

void fl_batuta_dowrite(t_fl_batuta *x, t_symbol *s)
{
	t_fourcc filetype = 'JSON', outtype;
	short numtypes = 1;
	char filename[MAX_FILENAME_CHARS];
	short path;

	strcpy(filename, "untitled.json");
	if (s == gensym("")) {      // if no argument supplied, ask for file
		saveas_promptset("choose a JSON file");
		if (saveasdialog_extended(filename, &path, &outtype, &filetype, numtypes)) {     // non-zero: user cancelled
			return;
		}
	}
	else {
		strcpy(filename, s->s_name);
		path = path_getdefault();
	}
	fl_batuta_writefile(x, filename, path);
}

void fl_batuta_writefile(t_fl_batuta *x, char *filename, short path)
{
	t_fourcc type = 'JSON';
	t_max_err err;
	t_filehandle fh;
	fl_tempo *ptempo;
	fl_tsign *ptsign;
	fl_goto *pgoto;
	fl_bar *pbar;
	fl_note *pnote;

	long total_bars;
	long total_tempos;
	long total_tsigns;
	long total_gotos;
	long total_notes_bar;
	long total_notes;
	
	t_atom *n_ap;
	long n_ac;
	char *text = (char *)sysmem_newptr(512 * sizeof(char));
	long textsize = 512;

	struct json_object *object, *atempos, *atsigns, *agotos, *anotes;
	struct json_object *tmp, *tmptempo, *tmptempoc_array, *tmptsign, *tmpgoto, *tmpnote;
	long count_buf = 0;

	long jj;

	err = path_createsysfile(filename, path, type, &fh);
	if (err) { object_error((t_object *)x, "couldn't create a JSON file"); return; }
	
	x->isloading = 1;
	
	//--------------------------------------------------------
	//bars
	object = json_object_new_object();
	total_bars = (long)linklist_getsize(x->l_bars);
	tmp = json_object_new_int64(total_bars);
	err = json_object_object_add(object, "bars", tmp);
	if (err) { object_error((t_object *)x, "write error: couldn't write bars"); sysfile_close(fh); return; }
	
	//--------------------------------------------------------
	//tempo
	atempos = json_object_new_array();
	total_tempos = (long)linklist_getsize(x->l_tempos);

	if (total_tempos > 0) {
		for (long i = 0; i < total_tempos; i++) {
			tmptempo = json_object_new_object();

			ptempo = linklist_getindex(x->l_tempos, i);
			if (!ptempo) { continue; }

			tmp = json_object_new_int64(ptempo->n_bar);
			json_object_object_add(tmptempo, "bar", tmp);
			tmp = json_object_new_double(ptempo->ms_beat);
			json_object_object_add(tmptempo, "msbeat", tmp);

			tmptempoc_array = json_object_new_array();
			tmp = json_object_new_double(ptempo->ms_inicio);
			json_object_array_add(tmptempoc_array, tmp);
			tmp = json_object_new_double(ptempo->ms_durvar);
			json_object_array_put_idx(tmptempoc_array, 1, tmp);
			tmp = json_object_new_double(ptempo->curva);
			json_object_array_put_idx(tmptempoc_array, 2, tmp);
			json_object_object_add(tmptempo, "curve", tmptempoc_array);

			if (!i) {
				json_object_array_add(atempos, tmptempo);
			}
			else {
				json_object_array_put_idx(atempos, i, tmptempo);
			}
		}
	}
	json_object_object_add(object, "tempo", atempos);

	//--------------------------------------------------------
	//tsign
	atsigns = json_object_new_array();
	total_tsigns = (long)linklist_getsize(x->l_tsigns);

	if (total_tsigns > 0) {
		for (long i = 0; i < total_tsigns; i++) {
			tmptsign = json_object_new_object();

			ptsign = linklist_getindex(x->l_tsigns, i);
			if (!ptsign) { continue; }

			tmp = json_object_new_int64(ptsign->n_bar);
			json_object_object_add(tmptsign, "bar", tmp);
			tmp = json_object_new_double(ptsign->beats);
			json_object_object_add(tmptsign, "beats", tmp);
			if (!i) {
				json_object_array_add(atsigns, tmptsign);
			}
			else {
				json_object_array_put_idx(atsigns, i, tmptsign);
			}
		}
	}
	json_object_object_add(object, "tsign", atsigns);

	//--------------------------------------------------------
	//goto
	agotos = json_object_new_array();
	total_gotos = (long)linklist_getsize(x->l_gotos);

	if (total_gotos > 0) {
		for (long i = 0; i < total_gotos; i++) {
			tmpgoto = json_object_new_object();

			pgoto = linklist_getindex(x->l_gotos, i);
			if (!pgoto) { continue; }

			tmp = json_object_new_int64(pgoto->n_bar);
			json_object_object_add(tmpgoto, "bar", tmp);
			tmp = json_object_new_int64(pgoto->to_bar);
			json_object_object_add(tmpgoto, "tobar", tmp);
			tmp = json_object_new_int64(pgoto->total_rep);
			json_object_object_add(tmpgoto, "rep", tmp);
			if (!i) {
				json_object_array_add(agotos, tmpgoto);
			}
			else {
				json_object_array_put_idx(agotos, i, tmpgoto);
			}
		}
	}
	json_object_object_add(object, "goto", agotos);

	//--------------------------------------------------------
	//note

	anotes = json_object_new_array();
	if (total_bars > 0) {
		
		total_notes = 0;
		for (long i = 0; i < total_bars; i++) {
			pbar = linklist_getindex(x->l_bars, i);
			total_notes += (long)linklist_getsize(pbar->notas);
		}

		if (total_notes > 0) {
			jj = 0;
			for (long i = 0; i < total_bars; i++) {
				pbar = linklist_getindex(x->l_bars, i);
				total_notes_bar = (long)linklist_getsize(pbar->notas);

				for (long j = 0; j < total_notes_bar; j++) {
					tmpnote = json_object_new_object();

					pnote = linklist_getindex(pbar->notas, j);
					n_ap = pnote->pnota;
					n_ac = pnote->cnota;
					err = atom_gettext(n_ac, n_ap, &textsize, &text, OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
					if (err) { object_error((t_object *)x, "write error: couldn't parse message from note"); continue; }

					tmp = json_object_new_int64(i);
					json_object_object_add(tmpnote, "bar", tmp);
					tmp = json_object_new_double(pnote->b_inicio);
					json_object_object_add(tmpnote, "start", tmp);
					tmp = json_object_new_int64(pnote->canal);
					json_object_object_add(tmpnote, "chan", tmp);
					tmp = json_object_new_string(text);
					json_object_object_add(tmpnote, "message", tmp);

					if (!jj) {
						json_object_array_add(anotes, tmpnote);
					}
					else {
						json_object_array_put_idx(anotes, jj, tmpnote);
					}
					jj++;
				}
			}
		}
	}
	json_object_object_add(object, "note", anotes);

	//--------------------------------------------------------
	long max_buf_len = x->max_buf_len;
	char *buf = sysmem_newptr(max_buf_len * sizeof(char));
	t_handle hbuf = sysmem_newhandle(0);

	count_buf = strlen(json_object_to_json_string_ext(object, JSON_C_TO_STRING_PRETTY)) + 1;
	if (count_buf > max_buf_len) {
		buf = (char *)sysmem_resizeptr(buf, count_buf * sizeof(char));
		if (!buf) { object_error((t_object *)x, "write error: out of memory for buffer"); }
	}
	strncpy_zero(buf, json_object_to_json_string_ext(object, JSON_C_TO_STRING_PRETTY), count_buf);
	sysmem_ptrandhand(buf, hbuf, count_buf * sizeof(char));

	err = sysfile_writetextfile(fh, hbuf, TEXT_LB_NATIVE);
	if (err) {
		sysmem_freeptr(buf);
		sysfile_close(fh);
		object_error((t_object *)x, "write error: couldn't write JSON file");
		return;
	}

	sysmem_freeptr(text);
	sysmem_freeptr(buf);
	err = sysfile_close(fh);
	if (err) { object_error((t_object *)x, "file closed with errors"); }
	else { object_post((t_object *)x, "file closed with no errors"); }
	
	x->isloading = 0;
}