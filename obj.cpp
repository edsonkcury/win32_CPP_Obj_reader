#include "obj.h"








bool Obj::ExtractDataToOpenGLBuffer(	unsigned int *_vb,int *_numTriangles,ptrGLFn_GenBuffer _pFunc	)
{

	unsigned int face_offset = 0;

	unsigned int l_numfaces = m_faces.size();
	/* Assume triangulated face. */
	if(l_numfaces==0)
	{ 
		printf("Error, 0 faces?\n");
		return false;
	}
	unsigned int l_num_faces = m_faces.size();//attrib.num_face_num_verts;
	unsigned int l_stride = 8; /*3 3 2*//* 9 = pos(3float), normal(3float), normal(2float) */
	unsigned int l_sidesperface = 3;
	unsigned int l_sizeOfOneVNT_bytes = sizeof(float) * l_stride;
	float* l_vntData = (float*)malloc( l_sizeOfOneVNT_bytes * l_sidesperface * l_num_faces);


	int sum=0;
	unsigned int l_allidxs = 0; 
	for (unsigned int i = 0; i < l_numfaces ; i++)
	{
		FaceTris &l_pface = m_faces[i];


		unsigned int l_numvtxs = l_pface.getNumVertices();//sempre sera 3, essa funcao nao suporta mais q 3 lados por poligono

		for (unsigned int k = 0; k < l_numvtxs ; k++)
		{
			unsigned int l_vtxId = l_pface.Getidx_Vertex(k);
			{
				v3D &v = m_V[l_vtxId];
				l_vntData[l_allidxs++]=v.x;
				l_vntData[l_allidxs++]=v.y;
				l_vntData[l_allidxs++]=v.z;

			}
			unsigned int l_NId = l_pface.Getidx_Normal(k);
			{
				v3D &v = m_N[l_NId];

				l_vntData[l_allidxs++]=v.x;
				l_vntData[l_allidxs++]=v.y;
				l_vntData[l_allidxs++]=v.z;
			}
			unsigned int l_TxId = l_pface.Getidx_Texel(k);
			{
				v3D &v = m_T[l_TxId];
				l_vntData[l_allidxs++]=v.x;
				l_vntData[l_allidxs++]=v.y;
			}
			sum++;
		}
	}
	//build a function as like:
	// 	*_vb = 0;
	// 	*_numTriangles = 0;
	// 	if (l_numfaces > 0) 
	// 	{
	// 		glGenBuffers(1, _vb);
	// 		glBindBuffer(GL_ARRAY_BUFFER, *_vb);
	// 		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) l_sizeOfOneVNT_bytes * l_sidesperface * l_num_faces,l_vb, GL_STATIC_DRAW);
	// 		*_numTriangles = (int)l_numfaces;
	// 	}

	*_numTriangles = (int)l_numfaces;
	(*_pFunc)(l_sizeOfOneVNT_bytes * l_sidesperface * l_num_faces,_vb,l_vntData);


	if(l_vntData)
		free(l_vntData);

	return true;
}

bool Obj::Parse()
{
	char *ptrBuf    = m_pBuffer;
	uint nCountFace = 0;
	uint nCountVtx  = 0;
	uint nCountNrm  = 0;
	uint nCountTxt  = 0;
	while(*ptrBuf!=EOF)
	{
		if(*ptrBuf=='#')//remover comentarios
			IgnoreData(&ptrBuf);
		if(*ptrBuf=='v' )
		{
			if(*(ptrBuf+1)=='n' && *(ptrBuf+2)==SPACE)//tipo normal
				ExtractType(&ptrBuf,NORMAL,nCountNrm++);
			else if(*(ptrBuf+1)=='t' && *(ptrBuf+2)==SPACE)//tipo textura
				ExtractType(&ptrBuf,TEXTURE,nCountTxt++);
			else if(*(ptrBuf+1)==SPACE)//tipo vertice
				ExtractType(&ptrBuf,VERTEX,nCountVtx++);
		}
		if(*ptrBuf=='f' && *(ptrBuf+1)==SPACE)
		{
			nCountFace++;
			if(hasFaceAttributes(ptrBuf)) 
			{
				//
				Obj::FaceTris l_pObjFaces; 

				char bufTmp[255];
				memset(bufTmp,0,sizeof(bufTmp));
				ptrBuf++;
				uint l_nTotalChars=0;
				uint numBars=0;
				while(true) 
				{	
					if(*ptrBuf==LINE_FEED)//
					{ 

						if(numBars==2) 
						{ 
							if(l_nTotalChars>0)
							{
								uint l_uiIndexOfNormal;
								sscanf(bufTmp,"%d",&l_uiIndexOfNormal);
								l_pObjFaces.addIndexOfNormals(l_uiIndexOfNormal-1); 
							}
							numBars=0;
						}
						l_nTotalChars=0;

						break; 
					}
					if(*ptrBuf==SPACE) 
					{
						if(numBars==2) 
						{ 
							if(l_nTotalChars>0) 
							{
								uint l_uiIndexOfNormal;
								sscanf(bufTmp,"%d",&l_uiIndexOfNormal); 
								l_pObjFaces.addIndexOfNormals(l_uiIndexOfNormal-1); 
							}
							numBars=0;
						}
						if(numBars==2)
							numBars=0;
						l_nTotalChars=0;
						memset(bufTmp,0,sizeof(bufTmp));
					}
					if(*ptrBuf==BAR)//PEGA O VERTICE
					{
						numBars++;
						if(numBars==1)
						{
							if(l_nTotalChars>0)
							{
								uint l_uiIndexOfVertex;
								sscanf(bufTmp,"%d",&l_uiIndexOfVertex);
								l_pObjFaces.addIndexOfVertex(l_uiIndexOfVertex-1);
							}
						}
						if(numBars==2)
						{
							if(l_nTotalChars>0)
							{
								uint l_uiIndexOfTexel;
								sscanf(bufTmp,"%d",&l_uiIndexOfTexel);
								l_pObjFaces.addIndexOfTexels(l_uiIndexOfTexel-1);
							}
						}
						l_nTotalChars=0;
						memset(bufTmp,0,sizeof(bufTmp));
					}
					if(IS_NUMBER(*ptrBuf))
						l_nTotalChars=GetOnlyNumbers(&ptrBuf,bufTmp);
					ptrBuf++;
				}
				m_faces.push_back(l_pObjFaces);
			}
		}
		ptrBuf++;
	}

	return true;
}
