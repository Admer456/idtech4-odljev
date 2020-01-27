#pragma once

class idModelExport {
private:
  void					Reset(void);
  bool					ParseOptions(idLexer &lex);
  int						ParseExportSection(idParser &parser);

  bool					ConvertMayaToMD5(void);
  static bool				initialized;

public:
  idStr					commandLine;
  idStr					src;
  idStr					dest;
  idStr         errorMessage;
  bool					force;

  idModelExport();

  static void				Shutdown(void);

  int						ExportDefFile(const char *filename);
  bool					ExportModel(const char *model);
  bool					ExportAnim(const char *anim);
  int						ExportModels(const char *pathname, const char *extension);
};