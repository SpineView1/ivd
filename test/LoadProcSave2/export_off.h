/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

/**
@name Save in OFF format
*/
//@{

#ifndef __VCGLIB_EXPORT_OFF
#define __VCGLIB_EXPORT_OFF

#include <stdio.h>
#include <wrap/io_trimesh/io_mask.h>
#include<wrap/io_trimesh/precision.h>
#include <vcg/complex/algorithms/clean.h>
#include <vcg/complex/algorithms/polygon_support.h>
#include <locale.h>


namespace vcg {
namespace tri {
namespace io {
template <class SaveMeshType>
class ExporterOFF
{

public:
    typedef typename SaveMeshType::VertexPointer VertexPointer;
    typedef typename SaveMeshType::ScalarType ScalarType;
    typedef typename SaveMeshType::VertexType VertexType;
    typedef typename SaveMeshType::FaceType FaceType;
    typedef typename SaveMeshType::FacePointer FacePointer;
    typedef typename SaveMeshType::VertexIterator VertexIterator;
    typedef typename SaveMeshType::FaceIterator FaceIterator;

    static char* SaveStream(SaveMeshType &m,int mask=0)
    {
        setlocale (LC_ALL,"");
        vcg::face::Pos<FaceType> he;
        vcg::face::Pos<FaceType> hei;

        std::string str;


        if( tri::HasPerVertexColor(m)  && (mask & io::Mask::IOM_VERTNORMAL))
            str.append("N");
        if( tri::HasPerVertexColor(m)   && (mask & io::Mask::IOM_VERTCOLOR))
            str.append("C");
        if( tri::HasPerVertexTexCoord(m) && (mask & io::Mask::IOM_VERTTEXCOORD))
            str.append("ST");
        str.append("OFF\n");

        int polynumber;
        if (mask &io::Mask::IOM_BITPOLYGONAL)
            polynumber = tri::Clean<SaveMeshType>::CountBitLargePolygons(m);
        else
            polynumber = m.fn;

        str.append(std::to_string(m.vn)).append(" ").append(std::to_string(polynumber)).append(" 0\n");//%d %d , , ); // note that as edge number we simply write zero

        //vertices
        int j;
        std::vector<int> FlagV;
        VertexPointer  vp;
        VertexIterator vi;
        const int DGT = vcg::tri::io::Precision<ScalarType>::digits();

        for(j=0,vi=m.vert.begin();vi!=m.vert.end();++vi)
        {
            vp=&(*vi);
            FlagV.push_back(vp->Flags()); // Save vertex flags
            if( ! vp->IsD() )
            {	// ***** ASCII *****

//                strcat(fpout,"%.*g %.*g %.*g " ,DGT,vp->P()[0],DGT,vp->P()[1],DGT,vp->P()[2]);
                str.append(std::to_string(vp->P()[0])).append(" ").append(std::to_string(vp->P()[1])).append(" ").append(std::to_string(vp->P()[2])).append(" ");
                if( tri::HasPerVertexColor(m)  && (mask & io::Mask::IOM_VERTCOLOR) )
                    str.append(std::to_string(vp->C()[0])).append(" ").append(std::to_string(vp->C()[1])).append(" ").append(std::to_string(vp->C()[2])).append(" ").append(std::to_string(vp->C()[3])).append(" ");
//                    strcat(fpout,"%d %d %d %d ",vp->C()[0],vp->C()[1],vp->C()[2],vp->C()[3] );

                if( tri::HasPerVertexNormal(m)  && (mask & io::Mask::IOM_VERTNORMAL) )
                    str.append(std::to_string((double) vp->N()[0])).append(" ").append(std::to_string((double) vp->N()[1])).append(" ").append(std::to_string((double) vp->N()[2])).append(" ");
//                    strcat(fpout,"%g %g %g ", double(vp->N()[0]),double(vp->N()[1]),double(vp->N()[2]));

                if( tri::HasPerVertexTexCoord(m)  && (mask & io::Mask::IOM_VERTTEXCOORD) )
                    str.append(std::to_string(vp->T().u())).append(" ").append(std::to_string(vp->T().v())).append(" ");
//                    strcat(fpout,"%g %g ",vp->T().u(),vp->T().v());

//                strcat(fpout,"\n");
                str.append("\n");


                vp->Flags()=j; // Trucco! Nascondi nei flags l'indice del vertice non deletato!
                j++;
            }
        }

        assert(j==m.vn);


        if (mask &io::Mask::IOM_BITPOLYGONAL) {
            tri::RequireFFAdjacency(m);
            std::vector<VertexPointer> polygon;
            tri::UpdateFlags<SaveMeshType>::FaceClearV(m);
            for(FaceIterator fi=m.face.begin();fi!=m.face.end();++fi) if (!fi->IsD()) if (!fi->IsV()) {
                vcg::tri::PolygonSupport<SaveMeshType,SaveMeshType>::ExtractPolygon(&*fi,polygon);
                if(!polygon.empty())
                {
//                    strcat(fpout,"%d ", int(polygon.size()) );
                    str.append(std::to_string(int(polygon.size())));
                    for (size_t i=0; i<polygon.size(); i++)
                            str.append(std::to_string(polygon[i]->Flags()));
//                            strcat(fpout,"%d ", polygon[i]->Flags() );
                    str.append("\n");
//                    strcat(fpout,"\n");
                }
            }
        }
        else {
            for(FaceIterator fi=m.face.begin();fi!=m.face.end();++fi)
            {
                if( ! fi->IsD() )
                {
                    if( tri::HasPerFaceColor(m)  && (mask & io::Mask::IOM_FACECOLOR) )
                        str.append("3 ").append(std::to_string(fi->cV(0)->Flags())).append(" ")
                                .append(std::to_string(fi->cV(1)->Flags())).append(" ")
                                .append(std::to_string(fi->cV(2)->Flags())).append(" ")
                                .append(std::to_string(fi->C()[0])).append(" ")
                                .append(std::to_string(fi->C()[1])).append(" ")
                                .append(std::to_string(fi->C()[2])).append("\n");
//                        strcat(fpout,"3 %d %d %d %i %i %i\n", fi->cV(0)->Flags(),	fi->cV(1)->Flags(), fi->cV(2)->Flags(), fi->C()[0],fi->C()[1],fi->C()[2] );
                    else
                        str.append("3 ").append(std::to_string(fi->cV(0)->Flags())).append(" ")
                                .append(std::to_string(fi->cV(1)->Flags())).append(" ")
                                .append(std::to_string(fi->cV(2)->Flags())).append("\n");

//                        strcat(fpout,"3 %d %d %d\n", fi->cV(0)->Flags(),	fi->cV(1)->Flags(), fi->cV(2)->Flags() );
                }
            }
        }


        // Recupera i flag originali
        j=0;
        for(vi=m.vert.begin();vi!=m.vert.end();++vi)
            (*vi).Flags()=FlagV[j++];

        size_t size = str.length();
        char * fpout = (char *) malloc(size);
        std::strcpy(fpout,str.c_str());
        return fpout;
    }


    static int Save(SaveMeshType &m, const char * filename, int mask=0 )
    {
        vcg::face::Pos<FaceType> he;
        vcg::face::Pos<FaceType> hei;
        FILE * fpout = fopen(filename,"w");
        if(fpout==NULL)	return 1; // 1 is the error code for cant'open, see the ErrorMsg function


        if( tri::HasPerVertexColor(m)  && (mask & io::Mask::IOM_VERTNORMAL))
            fprintf(fpout,"N");
        if( tri::HasPerVertexColor(m)   && (mask & io::Mask::IOM_VERTCOLOR))
            fprintf(fpout,"C");
        if( tri::HasPerVertexTexCoord(m) && (mask & io::Mask::IOM_VERTTEXCOORD))
            fprintf(fpout,"ST");
        fprintf(fpout,"OFF\n");

        int polynumber;
        if (mask &io::Mask::IOM_BITPOLYGONAL)
            polynumber = tri::Clean<SaveMeshType>::CountBitLargePolygons(m);
        else
            polynumber = m.fn;

        fprintf(fpout,"%d %d 0\n", m.vn, polynumber); // note that as edge number we simply write zero

        //vertices
        int j;
        std::vector<int> FlagV;
        VertexPointer  vp;
        VertexIterator vi;
        const int DGT = vcg::tri::io::Precision<ScalarType>::digits();

        for(j=0,vi=m.vert.begin();vi!=m.vert.end();++vi)
        {
            vp=&(*vi);
            FlagV.push_back(vp->Flags()); // Save vertex flags
            if( ! vp->IsD() )
            {	// ***** ASCII *****

                fprintf(fpout,"%.*g %.*g %.*g " ,DGT,vp->P()[0],DGT,vp->P()[1],DGT,vp->P()[2]);
                if( tri::HasPerVertexColor(m)  && (mask & io::Mask::IOM_VERTCOLOR) )
                    fprintf(fpout,"%d %d %d %d ",vp->C()[0],vp->C()[1],vp->C()[2],vp->C()[3] );

                if( tri::HasPerVertexNormal(m)  && (mask & io::Mask::IOM_VERTNORMAL) )
                    fprintf(fpout,"%g %g %g ", double(vp->N()[0]),double(vp->N()[1]),double(vp->N()[2]));

                if( tri::HasPerVertexTexCoord(m)  && (mask & io::Mask::IOM_VERTTEXCOORD) )
                    fprintf(fpout,"%g %g ",vp->T().u(),vp->T().v());

                fprintf(fpout,"\n");


                vp->Flags()=j; // Trucco! Nascondi nei flags l'indice del vertice non deletato!
                j++;
            }
        }

        assert(j==m.vn);


        if (mask &io::Mask::IOM_BITPOLYGONAL) {
            tri::RequireFFAdjacency(m);
            std::vector<VertexPointer> polygon;
            tri::UpdateFlags<SaveMeshType>::FaceClearV(m);
            for(FaceIterator fi=m.face.begin();fi!=m.face.end();++fi) if (!fi->IsD()) if (!fi->IsV()) {
                vcg::tri::PolygonSupport<SaveMeshType,SaveMeshType>::ExtractPolygon(&*fi,polygon);
                if(!polygon.empty())
                {
                    fprintf(fpout,"%d ", int(polygon.size()) );
                    for (size_t i=0; i<polygon.size(); i++) fprintf(fpout,"%d ", polygon[i]->Flags() );
                    fprintf(fpout,"\n");
                }
            }
        }
        else {
            for(FaceIterator fi=m.face.begin();fi!=m.face.end();++fi)
            {
                if( ! fi->IsD() )
                {
                    if( tri::HasPerFaceColor(m)  && (mask & io::Mask::IOM_FACECOLOR) )
                        fprintf(fpout,"3 %d %d %d %i %i %i\n", fi->cV(0)->Flags(),	fi->cV(1)->Flags(), fi->cV(2)->Flags(), fi->C()[0],fi->C()[1],fi->C()[2] );
                    else
                        fprintf(fpout,"3 %d %d %d\n", fi->cV(0)->Flags(),	fi->cV(1)->Flags(), fi->cV(2)->Flags() );
                }
            }
        }


        fclose(fpout);
        // Recupera i flag originali
        j=0;
        for(vi=m.vert.begin();vi!=m.vert.end();++vi)
            (*vi).Flags()=FlagV[j++];

        return 0;
    }

    static const char *ErrorMsg(int error)
    {
        static std::vector<std::string> off_error_msg;
        if(off_error_msg.empty())
        {
            off_error_msg.resize(2 );
            off_error_msg[0]="No errors";
            off_error_msg[1]="Can't open file";
        }

        if(error>1 || error<0) return "Unknown error";
        else return off_error_msg[error].c_str();
    }
    /*
            returns mask of capability one define with what are the saveable information of the format.
        */
    static int GetExportMaskCapability()
    {
        int capability = 0;
        capability |= vcg::tri::io::Mask::IOM_VERTCOORD;
        capability |= vcg::tri::io::Mask::IOM_VERTCOLOR;
        capability |= vcg::tri::io::Mask::IOM_VERTTEXCOORD;
        capability |= vcg::tri::io::Mask::IOM_FACEINDEX;
        capability |= vcg::tri::io::Mask::IOM_FACECOLOR;
        capability |= vcg::tri::io::Mask::IOM_BITPOLYGONAL;
        return capability;
    }

}; // end class
} // end namespace tri
} // end namespace io
} // end namespace vcg
//@}
#endif
