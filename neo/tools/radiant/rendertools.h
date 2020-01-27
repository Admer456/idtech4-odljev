
#include "../../renderer/ImmediateMode.h"


class fhTrisBuffer
{
public:
  fhTrisBuffer();
  void Add(const fhSimpleVert* vertices, int verticesCount);
  void Add(const fhSimpleVert& a, const fhSimpleVert& b, const fhSimpleVert& c);
  void Add(idVec3 a, idVec3 b, idVec3 c, idVec4 color = idVec4(1,1,1,1));
  void Clear();
  void Commit(idImage* texture, const idVec4& colorModulate, const idVec4& colorAdd);

  const fhSimpleVert* Vertices() const;
  int TriNum() const;

private:
  idList<fhSimpleVert> vertices;
};

class fhSurfaceBuffer
{
public:
  fhSurfaceBuffer();
  ~fhSurfaceBuffer();

  fhTrisBuffer* GetMaterialBuffer(const idMaterial* material);
  fhTrisBuffer* GetColorBuffer();

  void Clear();
  void Commit(const idVec4& colorModulate = idVec4(1,1,1,1), const idVec4& colorAdd = idVec4(0,0,0,0));

private:
  struct entry_t {
    const idMaterial* material;
    fhTrisBuffer trisBuffer;
  };

  idList<entry_t*> entries;
  fhTrisBuffer coloredTrisBuffer;
};

class fhPointBuffer
{
public:
  fhPointBuffer();
  ~fhPointBuffer();

  void Add(const idVec3& xyz, const idVec4& color, float size);
  void Add(const idVec3& xyz, const idVec3& color, float size);
  void Clear();
  void Commit();

private:
  struct entry_t {
    idList<fhSimpleVert> vertices;
    float size;
    void Commit();
  };

  idList<entry_t*> entries;
  const short* indices = nullptr;
};