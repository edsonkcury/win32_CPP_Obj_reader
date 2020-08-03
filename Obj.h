#ifndef Obj_h__
#define Obj_h__



#include <io.h>
#include <fcntl.h>
#include <vector>
#include <stdio.h>
typedef void (*ptrGLFn_GenBuffer)(int ,unsigned int *,float *); // type for conciseness

#define uint  unsigned int 
typedef  std::vector<uint>   uintv;

//simple struct representing 3D vertex
typedef struct  v3D
{ 
	float x,y,z;
public:
	v3D(float _x,float _y,float _z)
	{
		x= _x;
		y= _y;
		z= _z;
	};
};

class Obj
{

protected:
private:


#define  LINE_FEED  '\n'//0x0A //10d //em arquivos obj, line feed indica um novo dado
#define  SPACE      ' '//0x0D //13d
#define  BAR        '/'//0x0D //13d
#define IS_NUMBER(cr)   (cr>47 && cr<58)//trata-se de um caracter numero 0..9? se sim retorne 1

	//EStou experimentalmente tentando incrementar 
	//a posicao de um ponteiro de dentro de outra funcao
	void IgnoreData(char **ptr)
	{
		while(*(*ptr)!=LINE_FEED)
			*(*ptr)++;
	}

	void LoadToBuffer(char **_pBuff,char *_pBuffDest)
	{
		unsigned int i=0;
		while(*(*_pBuff)!=LINE_FEED)
			*(_pBuffDest+i++)=*(*_pBuff)++;
	}
	enum EXT_TYPES
	{
		VERTEX,NORMAL,TEXTURE
	};
	//extrai o dado conforme o flag type
	void ExtractType(char **_Buffer,//sem o endereço do buffer, nao poderiamos alterar a posicao do ponteiro
		unsigned char TYPE,
		uint  nVtxIndex)
	{
		char bufTmp[255];
		*(*_Buffer)++;
		*(*_Buffer)++;
		LoadToBuffer(_Buffer,bufTmp);
		float x=0.000f;
		float y=0.000f;
		float z=0.000f;

		if(TYPE == TEXTURE)
		{
			sscanf(bufTmp,"%f %f",&x,&y);
			m_T.push_back(v3D(x,y,0));
			return;
		}
		sscanf(bufTmp,"%f %f %f",&x,&y,&z);
		if(TYPE == VERTEX)
		{
			m_V.push_back(v3D(x,y,z));
			return;
		}
		if(TYPE == NORMAL)
		{
			m_N.push_back(v3D(x,y,z));
			return;
		}

	}

	uint GetOnlyNumbers( char **_pBuff,char *_pBuffDest )
	{
		uint i=0;
		while(true)
		{
			*(_pBuffDest+i++)=*(*_pBuff)++;
			if(!IS_NUMBER(*(*_pBuff)))
			{
				*(*_pBuff)--;
				return i;
			}
		}
		return i;
	}

	bool Parse();

	bool Load(char *_szObjFileName)
	{
		bool ret=false;
		ret = ReadObjToBuffer(_szObjFileName);
		if(!ret)
		{
			printf("***ReadObjet Error!\n");
			return false;
		}
		printf("*Begin Parsing %s File...\n",_szObjFileName);
		ret = Parse();
		if(!ret)
		{
			printf(" Error Parsing file ret=false\n");
			return false;
		}
		if(m_pBuffer!=NULL)//o buffer foi alocado, desaloc
		{
			free(m_pBuffer);
			m_pBuffer=NULL;
		}
		return ret;
	}



	// +-----------face
	// | +---------indice do vertice
	// | | +-------indice do texel
	// | | | +-----indice do normal
	// | | | |
	// V V V V |iden||iden|
	// f 1/2/3 2/3/4 3/4/5
	// f v/vt/vn


	bool hasFaceAttributes(char *ptrBuf)
	{
		char *l_ptrParse = ptrBuf;
		while(*l_ptrParse!='\n')
		{
			if(*l_ptrParse=='/')
				return true;
			l_ptrParse++;
		}
		return false;
	}

	//Lê o arquivo obj em raw data e carrega no buffer
	bool ReadObjToBuffer(char *szFileName)
	{
		int dfile;
		dfile = _open( szFileName, _O_RDONLY );
		if( dfile == -1 )
		{
			printf( "Open failed on input file");
			return false;
		}
		int nr = _filelength(dfile);
		m_pBuffer = (char*)malloc(nr+1);//new char[nr+1];
		memset(m_pBuffer,EOF,sizeof(char)*nr+1);
		int l_nTotalReaded=_read(dfile,m_pBuffer,nr);

		_close(dfile);
		m_pBuffer[l_nTotalReaded] = EOF;
		printf("Total bytes readed:%d\n",l_nTotalReaded);
		return true;
	}




public:

	struct FaceTris
	{
	private:

		uintv m_idxOfVertex;//indices de todos os vertices, o size deset vetor nosdá o numero de lados deste poligono
		uintv m_idxOfNormals;//indices de  todos os normais
		uintv m_idxOfTexels;//indies de todos os texels desta face
	public:


		__inline uint Getidx_Normal(uint n){return m_idxOfNormals[n];};
		__inline uint Getidx_Vertex(uint n){return m_idxOfVertex[n];};
		__inline uint Getidx_Texel(uint n){return m_idxOfTexels[n];};

		__inline void addIndexOfVertex(uint n){m_idxOfVertex.push_back(n);}
		__inline void addIndexOfNormals(uint n){m_idxOfNormals.push_back(n);}
		__inline void addIndexOfTexels(uint n){m_idxOfTexels.push_back(n);}
		uint getNumVertices(){return m_idxOfVertex.size();}
	protected:

	};
	typedef  std::vector<FaceTris>   ArrFace;//todas as faces
	typedef  std::vector<v3D>  ArrData;//todos vertices

	ArrFace             m_faces;//todas as faces deste objeto
	ArrData             m_V,m_T,m_N;
	char               *m_pBuffer;


	Obj( ):m_pBuffer(NULL)
	{
	};

	bool LoadFileObj(char *_szObjFileName)
	{
		return Load(_szObjFileName);
	};


	//ExtractDataToOpenGLBuffer:
	//Extract all data from this object to an openGL buffer
	//*_vb (write) name of vt buffer
	//*_numTriangles number of tris this 
	//_pFunc pointer of function for save data extract to buffer opengl generated 
	bool ExtractDataToOpenGLBuffer(unsigned int *_vb,int *_numTriangles,ptrGLFn_GenBuffer _pFunc	);
	

	~Obj()
	{
		if(m_pBuffer)
			free(m_pBuffer);
	};



};

#endif // Obj_h__