#include "fl_batuta~.h"

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
	t_fourcc filetype = 'DATA', outtype;
	short numtypes = 1;
	char filename[MAX_PATH_CHARS];
	short path;
	if (s == gensym("")) {      // if no argument supplied, ask for file
		open_promptset("choose a data file");
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
	// we have a file
	//object_post((t_object *)x, "archivo %s encontrado", filename);
	fl_batuta_openfile(x, filename, path);
}

void fl_batuta_openfile(t_fl_batuta *x, char *filename, short path)
{
	t_filehandle fh;
	t_max_err err = MAX_ERR_NONE;

	t_ptr_size count_buf = 0;

	long total_bars;
	fl_bar *pbar;
	long total_notas;
	fl_note *pnota;

	long rbar;
	long rtotal_tempos;
	long rtotal_tsigns;
	long rtotal_gotos;
	long rtotal_bars;
	long rtotal_notes;
	float rt_curve;
	float rt_msbeat;
	float rt_msdurvar;
	float rt_msstart;
	short rt_type;
	float rts_beats;
	long rgt_reps;
	long rgt_tobar;
	float rn_start;
	long rn_chan;
	long rn_ac;
	t_atom *rn_ap = NULL;

	if (path_opensysfile(filename, path, &fh, READ_PERM)) {
		object_error((t_object *)x, "error opening %s", filename);
		return;
	}

	x->isloading = 1;

	//clear lists
	total_bars = (long)linklist_getsize(x->l_bars);
	for (int i = 0; i < total_bars; i++) {
		pbar = linklist_getindex(x->l_bars, i);
		total_notas = (long)linklist_getsize(pbar->notas);
		for (int k = 0; k < total_notas; k++) {
			pnota = linklist_getindex(pbar->notas, k);
			sysmem_freeptr(pnota->pnota);
		}
		object_free(pbar->notas);
	}
	linklist_clear(x->l_bars);
	linklist_clear(x->l_tempos);
	linklist_clear(x->l_tsigns);
	linklist_clear(x->l_gotos);

	/*tempos-----------------------------------------------------------------------*/
	//|num tempos| n x [curve(float), ms_beat(float), ms_durvar(float), ms_inicio(float), n_bar(long), type(short)]
	count_buf = sizeof(long);
	err = sysfile_read(fh, &count_buf, &rtotal_tempos);
	if (err) { rtotal_tempos = -1; }
	if (rtotal_tempos > 0) {
		
		for (int i = 0; i < rtotal_tempos; i++) {
			count_buf = sizeof(float);
			err = sysfile_read(fh, &count_buf, &rt_curve);
			//count_buf = sizeof(float);
			err += sysfile_read(fh, &count_buf, &rt_msbeat);
			//count_buf = sizeof(float);
			err += sysfile_read(fh, &count_buf, &rt_msdurvar);
			//count_buf = sizeof(float);
			err += sysfile_read(fh, &count_buf, &rt_msstart);
			count_buf = sizeof(long);
			err += sysfile_read(fh, &count_buf, &rbar);
			count_buf = sizeof(short);
			err += sysfile_read(fh, &count_buf, &rt_type);

			if (err) { object_error((t_object *)x, "read error: couldn't read tempos"); sysfile_close(fh); return; }

			err = do_add_tempo(x, rt_type, rbar, rt_msstart, rt_msbeat, rt_msdurvar, rt_type);
			if(err){ object_error((t_object *)x, "read error: tempo couldn't be added"); }
		}
	}
	else { err = do_add_tempo(x, 0, 0, 0., 500., 0., 0.); }
	
	//tsigns-----------------------------------------------------------------------/
	//|num tsigns| n x [n bar(long), beats(float)]
	count_buf = sizeof(long);
	err = sysfile_read(fh, &count_buf, &rtotal_tsigns);
	if (err) { rtotal_tsigns = -1; }
	if (rtotal_tsigns > 0) {
	
		for (int i = 0; i < rtotal_tsigns; i++) {
			count_buf = sizeof(long);
			err = sysfile_read(fh, &count_buf, &rbar);
			count_buf = sizeof(float);
			err += sysfile_read(fh, &count_buf, &rts_beats);

			if (err) { object_error((t_object *)x, "read error: couldn't read tempos"); sysfile_close(fh); return; }

			err = do_add_signature(x, rbar, rts_beats);
			if (err) { object_error((t_object *)x, "read error: tsign couldn't be added"); }
		}
	}
	else { do_add_signature(x, 0, 4.); }
	
	//gotos------------------------------------------------------------------------/
	//|n gotos| n x [n bar(long), total rep(long), to bar(long)]
	count_buf = sizeof(long);
	err = sysfile_read(fh, &count_buf, &rtotal_gotos);
	if (err) { rtotal_gotos = -1; }
	if (rtotal_gotos > 0) {
		
		for (int i = 0; i < rtotal_gotos; i++) {
			count_buf = sizeof(long);
			err = sysfile_read(fh, &count_buf, &rbar);
			//count_buf = sizeof(long);
			err += sysfile_read(fh, &count_buf, &rgt_reps);
			//count_buf = sizeof(long);
			err += sysfile_read(fh, &count_buf, &rgt_tobar);

			if (err) { object_error((t_object *)x, "read error: couldn't read gotos"); sysfile_close(fh); return; }

			err = do_add_goto(x, rbar, rgt_tobar, rgt_reps);
			if (err) { object_error((t_object *)x, "read error: goto couldn't be added"); }
		}
	}

	//notes------------------------------------------------------------------------/
	//|total bars|total notes| n * [nbar, start(float), canal(long), listac(long), listav(atom *)]|
	count_buf = sizeof(long);
	err = sysfile_read(fh, &count_buf, &rtotal_bars);
	if (err) { rtotal_bars = -1; }
	if (rtotal_bars > 0) {

		for (int i = 0; i < rtotal_bars; i++) {
			err = do_add_bar(x, 0);
			if (err) { object_error((t_object *)x, "read error: bar couldn't be added"); sysfile_close(fh); return; }
		}

		count_buf = sizeof(long);
		err = sysfile_read(fh, &count_buf, &rtotal_notes);
		if (err) { rtotal_notes = -1; }
		if (rtotal_notes > 0) {

			for (int i = 0; i < rtotal_notes; i++) {
				count_buf = sizeof(long);
				err = sysfile_read(fh, &count_buf, &rbar);
				count_buf = sizeof(float);
				err += sysfile_read(fh, &count_buf, &rn_start);
				count_buf = sizeof(long);
				err += sysfile_read(fh, &count_buf, &rn_chan);
				//count_buf = sizeof(long);
				err += sysfile_read(fh, &count_buf, &rn_ac);
				
				count_buf = rn_ac * sizeof(t_atom);
				if(!rn_ap){ rn_ap = (t_atom *)sysmem_newptr(rn_ac * sizeof(t_atom)); }
				else { rn_ap = (t_atom *)sysmem_resizeptr(rn_ap, rn_ac * sizeof(t_atom)); }
				if (!rn_ap) { err = -1; }
				else { err += sysfile_read(fh, &count_buf, rn_ap); }

				if (err) { object_error((t_object *)x, "read error: couldn't read notes"); sysfile_close(fh); return; }

				err = do_add_note(x, rbar, rn_start, rn_chan, rn_ac, rn_ap);
				if (err) { object_error((t_object *)x, "read error: note couldn't be added"); }
			}
		}
	}
	else { do_add_bar(x, 0); }

	if (rn_ap) { sysmem_freeptr(rn_ap); }

	sysfile_close(fh);

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
	t_fourcc filetype = 'DATA', outtype;
	short numtypes = 1;
	char filename[MAX_FILENAME_CHARS];
	short path;

	strcpy(filename, "untitled");
	if (s == gensym("")) {      // if no argument supplied, ask for file
		saveas_promptset("choose a data file");
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
	t_fourcc type = 'DATA';
	t_max_err err;
	t_filehandle fh;
	fl_tempo *ptempo;
	fl_tsign *ptsign;
	fl_goto *pgoto;
	fl_bar *pbar;
	fl_note *pnote;
	t_atom *patom;
	long total_tempos;
	long total_tsigns;
	long total_gotos;
	long total_notes_all;
	long notas_bar;
	long total_bars;
	long lnumof = 0;
	short snumof = 0;
	float fnumof = 0.f;
	t_ptr_size count_buf = 0;

	err = path_createsysfile(filename, path, 'DATA', &fh);
	if (err) { return; }
	
	x->isloading = 1;
	
	/*tempos-----------------------------------------------------------------------*/
	//num tempos: n x [curve(float), ms_beat(float), ms_durvar(float), ms_inicio(float), n_bar(long), type(short)]
	count_buf = sizeof(long);
	total_tempos = lnumof = (long)linklist_getsize(x->l_tempos);
	err = sysfile_write(fh, &count_buf, &lnumof);
	if (err) { object_error((t_object *)x, "write error: couldn't write tempos"); sysfile_close(fh); return; }
	if (total_tempos > 0) {
		for (int i = 0; i < total_tempos; i++) {
			ptempo = linklist_getindex(x->l_tempos, i);
			//curve(float)
			count_buf = sizeof(float);
			fnumof = ptempo->curva;
			err = sysfile_write(fh, &count_buf, &fnumof);
			//ms_beat(float)
			//count_buf = sizeof(float);
			fnumof = ptempo->ms_beat;
			err += sysfile_write(fh, &count_buf, &fnumof);
			//ms_durvar(float)
			//count_buf = sizeof(float);
			fnumof = ptempo->ms_durvar;
			err += sysfile_write(fh, &count_buf, &fnumof);
			//ms_inicio(float)
			//count_buf = sizeof(float);
			fnumof = ptempo->ms_inicio;
			err += sysfile_write(fh, &count_buf, &fnumof);
			//n_bar(long)
			count_buf = sizeof(long);
			lnumof = ptempo->n_bar;
			err += sysfile_write(fh, &count_buf, &lnumof);
			//type(short)
			count_buf = sizeof(short);
			snumof = ptempo->type;
			err += sysfile_write(fh, &count_buf, &snumof);

			if (err) { object_error((t_object *)x, "write error: couldn't write tempos"); sysfile_close(fh); return; }
		}
	}
	
	//time signature----------------------------------------------------------/
	//|num tsigns| n x [n bar(long), beats(float)]
	count_buf = sizeof(long);
	total_tsigns = lnumof = (long)linklist_getsize(x->l_tsigns);
	err = sysfile_write(fh, &count_buf, &lnumof);
	if (err) { object_error((t_object *)x, "write error: couldn't write tsigns"); sysfile_close(fh); return; }
	if (total_tsigns > 0) {
		for (int i = 0; i < total_tsigns; i++) {
			//n bar(long)
			ptsign = linklist_getindex(x->l_tsigns, i);
			count_buf = sizeof(long);
			lnumof = ptsign->n_bar;
			err = sysfile_write(fh, &count_buf, &lnumof);
			//beats(float)	
			count_buf = sizeof(float);
			fnumof = ptsign->beats;
			err += sysfile_write(fh, &count_buf, &fnumof);

			if (err) { object_error((t_object *)x, "write error: couldn't write tsigns"); sysfile_close(fh); return; }
		}
	}

	//gotos-------------------------------------------------------------------/
	//|n gotos| n x [n bar(long), total rep(long), to bar(long)]
	count_buf = sizeof(long);
	total_gotos = lnumof = (long)linklist_getsize(x->l_gotos);
	err = sysfile_write(fh, &count_buf, &lnumof);
	if (err) { object_error((t_object *)x, "write error: couldn't write gotos"); sysfile_close(fh); return; }
	if (total_gotos > 0) {
		for (int i = 0; i < total_gotos; i++) {
			pgoto = linklist_getindex(x->l_gotos, i);
			// n bar(long)
			count_buf = sizeof(long);
			lnumof = pgoto->n_bar;
			err = sysfile_write(fh, &count_buf, &lnumof);
			// total rep(long)
			//count_buf = sizeof(long);
			lnumof = pgoto->total_rep;
			err += sysfile_write(fh, &count_buf, &lnumof);
			// to bar(long)
			//count_buf = sizeof(long);
			lnumof = pgoto->to_bar;
			err += sysfile_write(fh, &count_buf, &lnumof);

			if (err) { object_error((t_object *)x, "write error: couldn't write gotos"); sysfile_close(fh); return; }
		}
	}

	//notes------------------------------------------------------------------------/
	//|total bars|total notes| n * [nbar, start(float), canal(long), listac(long), listav(atom)]|
	count_buf = sizeof(long);
	total_bars = lnumof = (long)linklist_getsize(x->l_bars);
	err = sysfile_write(fh, &count_buf, &lnumof); //total bars
	if (err) { object_error((t_object *)x, "write error: couldn't write notes"); sysfile_close(fh); return; }
	if (total_bars > 0) {

		total_notes_all = 0;
		notas_bar = 0;

		for (int i = 0; i < total_bars; i++) {
			pbar = linklist_getindex(x->l_bars, i);
			total_notes_all += (long)linklist_getsize(pbar->notas);
		}

		//total notes
		//count_buf = sizeof(long);
		lnumof = total_notes_all;
		err = sysfile_write(fh, &count_buf, &lnumof); 
		if (err) { object_error((t_object *)x, "write error: couldn't write notes"); sysfile_close(fh); return; }
		if (total_notes_all > 0) {
			
			long accum = 0;
			for (int i = 0; i < total_bars; i++) {
				pbar = linklist_getindex(x->l_bars, i);
				notas_bar = (long)linklist_getsize(pbar->notas);

				for (int j = 0; j < notas_bar; j++) {
					pnote = linklist_getindex(pbar->notas, j);
					//n bar
					count_buf = sizeof(long);
					lnumof = i;
					err = sysfile_write(fh, &count_buf, &lnumof);
					//beat start
					count_buf = sizeof(float);
					fnumof = pnote->b_inicio;
					err += sysfile_write(fh, &count_buf, &fnumof);
					//chan
					count_buf = sizeof(long);
					lnumof = pnote->canal;
					err += sysfile_write(fh, &count_buf, &lnumof);
					//ac
					//count_buf = sizeof(long);
					lnumof = pnote->cnota;
					err += sysfile_write(fh, &count_buf, &lnumof);
					//av
					count_buf = pnote->cnota * sizeof(t_atom);
					patom = pnote->pnota;
					err += sysfile_write(fh, &count_buf, patom);

					if (err) { object_error((t_object *)x, "write error: couldn't write notes"); sysfile_close(fh); return; }
				}
			}
		}
	}

	sysfile_close(fh);

	object_post((t_object *)x, "file saved with no errors");
	x->isloading = 0;
}