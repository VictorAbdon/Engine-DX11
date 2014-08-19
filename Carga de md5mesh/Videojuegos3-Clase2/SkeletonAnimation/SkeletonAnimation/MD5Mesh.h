#include <iostream>
#include <fstream>
#include <string>
//para usar los vectores
#include <D3D11.h>//graficos
#include <D3Dx11.h>//core de directx

#include <vector>
#include <xnamath.h>//para matematicas

#include "Util.h"

using namespace std;

struct Joint{
	string name;
	int parent;
	XMFLOAT3 position;
	XMFLOAT4 orientation;

};

struct Vertex{
	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;

	int vertexIndex;
	int startWeight;
	int countWeight;
	int timesUsed;
};

struct Weight{
	int weightIndex;
	int joint;
	float bias;
	XMFLOAT3 position;
};

struct Triangle{
	int triangleIndex;
	int vertexIndices[3];
};


struct Mesh{
	string shader;//textura
	int numVertices;
	int numTriangles;
	int numWeights;
	vector<Vertex> vertices;
	vector<Weight> weights;
	vector<Triangle> triangles;
	vector<int> indices;

	~Mesh(){
		vertices.clear();
		weights.clear();
		triangles.clear();
	}
};

class MD5Mesh{

private:
	string filename;
	int numJoints;
	int numMeshes;
	vector<Joint> joints;
	vector<Mesh> meshes;

public:
	MD5Mesh(string filename){
		this->filename = filename;
		ifstream fileStream(filename, ifstream::in);

		if (fileStream.is_open()){

			readNumJointsAndMeshes(fileStream);
			readJoints(fileStream);
			readMeshes(fileStream);
		}
	}

	void readNumJointsAndMeshes(ifstream &fileStream){
		string currentLine;
		
		while (getline(fileStream,currentLine)){

			string *currentLineSplitted = new string[2];
			SplitString(currentLine,2,currentLineSplitted);//HAY QUE HACERLA, objeto a leer, cuantas partes, en donde guardar

			if (currentLineSplitted[0] == "numJoints"){
				this->numJoints = atoi(currentLineSplitted[1].c_str());
			}
			else if (currentLineSplitted[0] == "numMeshes"){
				this->numMeshes = atoi(currentLineSplitted[1].c_str());
				return;

			}

		}
	}

	void readJoints(ifstream &fileStream){
		string currentLine;
		bool isJointsZone = false;//para saber si leo el area del archivo que corresponde a joints

		int i = 0;//Cuantos joints he leido

		while (getline(fileStream, currentLine) && i < this->numJoints){

			if (isJointsZone){
				Joint joint;
				string *currentLineSplitted = new string[12];
				SplitString(currentLine, 12, currentLineSplitted);

				joint.name = currentLineSplitted[0];
				TrimString(joint.name, "\"");//funcion del Util.h
				joint.parent = atoi(currentLineSplitted[1].c_str());
				joint.position.x = atof(currentLineSplitted[3].c_str());//3 por el parentesis en el 2
				joint.position.z = atof(currentLineSplitted[4].c_str());//Y por que su y es nuestra z
				joint.position.y = atof(currentLineSplitted[5].c_str());//z

				joint.orientation.x = atof(currentLineSplitted[8].c_str());
				joint.orientation.z = atof(currentLineSplitted[9].c_str());
				joint.orientation.y = atof(currentLineSplitted[10].c_str());
				joint.orientation.w = GetWComponent(joint.orientation);//del UTIL

				joints.push_back(joint);
				i++;
			}
			else
			{
				if (currentLine == "joints {"){
					isJointsZone = true;

				}
			}
		}

	}

	void readMeshes(ifstream &fileStream){
		string currentLine;
		bool isMeshesZone = false;//para saber si leo el area del archivo que corresponde a meshes
		
		
		
		for (int i = 0; i < this->numMeshes; i++){
		//leo todos los meshes
			//declaro el mesh actual
			Mesh mesh;

			while (getline(fileStream, currentLine))
			{

				if (isMeshesZone)
				{
					if (currentLine == "}"){
						this->meshes.push_back(mesh);//Subo el mesh cargado
						break;//termine el mesh
					}
					else
					{
						string *currentLineSplitted;
						
						//me brinco el comentario
						getline(fileStream, currentLine);//linea que sigue del comentario
						//ahora cargo el "shader"/textura

						currentLineSplitted = new string[2];//para el shader o algo de 2 elementos

						//guardo shader
						SplitString(currentLine, 2, currentLineSplitted);
						//currentLineSplitted[1];//aqui esta el shader
						mesh.shader = currentLineSplitted[1];//paso el shader
						TrimString(mesh.shader, "\"");//quitar comillas
						
						//salto linea
						getline(fileStream, currentLine);//sig linea que tiene nada
						getline(fileStream, currentLine);//brinco a la linea que tiene num verts

						//leo numero de vertices
						SplitString(currentLine, 2, currentLineSplitted);
						currentLineSplitted[1];//aqui esta el num verts
						mesh.numVertices = atoi(currentLineSplitted[1].c_str());

						currentLineSplitted = new string[8];//para 8 elementos del vertice
						for (int i = 0; i < mesh.numVertices; i++)
						{
							getline(fileStream, currentLine);
							SplitString(currentLine, 8, currentLineSplitted);
							//currentLineSplitted[1];//aqui va el vertice ahora

							Vertex vertice;
							vertice.vertexIndex = atoi(currentLineSplitted[1].c_str());

							vertice.uv = XMFLOAT2(atof(currentLineSplitted[3].c_str()), atof(currentLineSplitted[4].c_str()));
							vertice.startWeight = atof(currentLineSplitted[6].c_str());
							vertice.countWeight = atof(currentLineSplitted[7].c_str());
							mesh.vertices.push_back(vertice);//cargo el vertice en el mesh

						}//termine de cargar los meshes

						getline(fileStream, currentLine);//leo espacio
						getline(fileStream, currentLine);//leo num tris

						currentLineSplitted = new string[2];//para 2 elementos, num tris
						SplitString(currentLine, 2, currentLineSplitted);//parto
						mesh.numTriangles = atoi(currentLineSplitted[1].c_str());

						//for de triangulos
						currentLineSplitted = new string[5];//para 5 elementos del triangulo
						for (int i = 0; i < mesh.numTriangles; i++)
						{
							getline(fileStream, currentLine);
							SplitString(currentLine, 5, currentLineSplitted);
							//currentLineSplitted[1];//aqui va el triangulo ahora

							Triangle triangulo;
							triangulo.triangleIndex = atoi(currentLineSplitted[1].c_str());

							triangulo.vertexIndices[0] = atoi(currentLineSplitted[2].c_str());
							triangulo.vertexIndices[1] = atoi(currentLineSplitted[3].c_str());
							triangulo.vertexIndices[2] = atoi(currentLineSplitted[4].c_str());

							mesh.triangles.push_back(triangulo);//cargo el triangulo en el mesh

						}//termine de cargar los triangulos


						currentLineSplitted = new string[2];
						getline(fileStream, currentLine);//leo espacio
						getline(fileStream, currentLine);//leo num weights
						SplitString(currentLine, 2, currentLineSplitted);//parto
						mesh.numWeights = atoi(currentLineSplitted[1].c_str());

						//for de pesos(weight)
						currentLineSplitted = new string[9];//para 9 elementos del weight
						for (int i = 0; i < mesh.numWeights; i++)
						{
							getline(fileStream, currentLine);
							SplitString(currentLine, 9, currentLineSplitted);
							currentLineSplitted[1];//aqui va el weight ahora

							Weight weight;
							weight.weightIndex = atoi(currentLineSplitted[1].c_str());
							weight.joint = atoi(currentLineSplitted[2].c_str());
							weight.bias = atof(currentLineSplitted[3].c_str());
							weight.position = XMFLOAT3( atof(currentLineSplitted[5].c_str()), atof(currentLineSplitted[6].c_str()), atof(currentLineSplitted[7].c_str()) );

							
							mesh.weights.push_back(weight);//cargo el triangulo en el mesh

						}//termine de cargar los weight
						int x = 0;
						

					}
				}
				else if (currentLine == "mesh {"){
						isMeshesZone = true;//estoy en un mesh
				}

			}
			isMeshesZone = false;//termine meshes
			
		}
		this->meshes;
		int z = 1;
		
	}

};