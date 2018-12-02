## Osteotomy
Project developed under the discipline of Computer Vision of the Master's Degree in Informatics Engineering (University of Minho).

This project consists in the development of an X-ray processing tool. The aim is to assist the physician's decision on how to perform a Varus Osteotomy of the knee with the information provided by the patient radiographs. 
For this purpose it was used the OpenCV library for C++, in order to perform the image treatment and object recognition. The automatic detection of points of interest is based on the Hough generalized transform with the aid of contour detection of the Canny algorithm. Finally, the graphical interface was implemented using the C++ Qt framework.

*Used Technologies: C++, OpenCV, Qt, Qt Creator, DICOM, GitHub, Trello, LaTeX, Texmaker, among others.*

![Interface](https://raw.githubusercontent.com/david-branco/osteotomy/master/screenshots/manual.png)<br>

[More Screenshots](https://github.com/david-branco/osteotomy/tree/master/screenshots)

The system has among its main functionalities:
- Load DICOM files;
- Minimalist visualization of meta data from the DICOM file;
- Manual and automatic calculation of key points;
- Manual adjustment of key points;
- Record results in image format;
- Calculation of the angle for the 2 legs (one at a time);
- Calculation and suggestion of cutting;
- Minimal representation of the leg after surgery;
- Average automatic calculation time of approximately 1 second (with 3520x4280 images);
- Configurable parameters;
- Graphic interface;
- Free Zoom;
- Among other features.

---

## Osteotomia
Projecto desenvolvido no âmbito da disciplina de Visão por Computador do Mestrado em Engenharia Informática (Universidade do Minho).

Este projecto consiste no desenvolvimento de uma ferramenta de processamento de radiografias. O objectivo é auxiliar a decisão do médico sobre como proceder uma Osteotomia Varus do joelho com a informação fornecida pelas radiografias de pacientes.
Para tal foi utilizada a biblioteca OpenCV para C++, de modo a realizar o tratamento de imagem e reconhecimento de objectos. A detecção automática dos pontos de interesse baseia-se na transformada generalizada de Hough com o auxílio da detecção de contornos do algoritmo Canny. Por fim, a interface gráfica foi implementada utilizando a framework Qt de C++.

*Tecnologias Utilizadas: C++, OpenCV, Qt, Qt Creator, DICOM, GitHub, Trello, LaTeX, Texmaker, entre outras.*

![Interface](https://github.com/david-branco/osteotomy/tree/master/screenshotsmanual.png)<br>

[Mais Screenshots](https://raw.githubusercontent.com/david-branco/osteotomy/master/screenshots/)

O sistema possui entre as suas funcionalidades principais:
- Carregar ficheiros DICOM;
- Visualização minimalista de meta dados do ficheiro DICOM;
- Cálculo manual e automático de pontos-chave;
- Ajuste manual de pontos-chave;
- Gravar resultados em formato de imagem;
- Cálculo do ângulo para as 2 pernas (uma de cada vez);
- Cálculo e sugestão de corte;
- Representação minimalista da perna após cirurgia;
- Tempo médio de cálculo automático de aproximadamente 1 segundo (com imagens 3520x4280);
- Parâmetros configuráveis;
- Interface gráfica;
- Zoom Livre;
- Entre outras funcionalidades.