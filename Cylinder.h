#ifndef CYLINDER_H
#define CYLINDER_H

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

#include <vector>
#include <string>

using namespace std;
using namespace glm;

#define PI 3.1416

class Cylinder {
public:
	Cylinder() {

	}
	Cylinder(float topRadius, float baseRadius, float height,
		float stackCount, float sectorCount,
		vec3 ambient, vec3 diffuse, vec3 specular, float shininess,
		unsigned int texture0, unsigned int texture1, unsigned int texture2) {
		this->topRadius = topRadius, this->baseRadius = baseRadius, this->height = height;
		this->stackCount = stackCount, this->sectorCount = sectorCount;
		this->ambient = ambient, this->diffuse = diffuse, this->specular = specular, this->shininess = shininess;
		this->texture0 = texture0, this->texture1 = texture1, this->texture2 = texture2;
		this->generateData();
		this->bindData();
	}
	void drawTexture(Shader& shader, mat4 model) const {

		shader.use();

		shader.setMat4("model", model);
		shader.setFloat("material.shininess", this->shininess);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->texture0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, this->texture1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, this->texture2);

		glBindVertexArray(VAO);

		shader.setInt("material.diffuse", 0);
		shader.setInt("material.specular", 0);
		glDrawElements(GL_TRIANGLES, textureCount0, GL_UNSIGNED_INT, (void*)0);

		shader.setInt("material.diffuse", 1);
		shader.setInt("material.specular", 1);
		glDrawElements(GL_TRIANGLES, textureCount1 - textureCount0, GL_UNSIGNED_INT, (void*)(textureCount0 * sizeof(unsigned int)));

		shader.setInt("material.diffuse", 2);
		shader.setInt("material.specular", 2);
		glDrawElements(GL_TRIANGLES, textureCount2 - textureCount1, GL_UNSIGNED_INT, (void*)(textureCount1 * sizeof(unsigned int)));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		glUseProgram(0);
	}
	void drawColor(Shader& shader, mat4 model) const {

		shader.use();

		shader.setMat4("model", model);
		shader.setVec3("material.ambient", this->ambient);
		shader.setVec3("material.diffuse", this->diffuse);
		shader.setVec3("material.specular", this->specular);
		shader.setFloat("material.shininess", this->shininess);

		glBindVertexArray(VAO);

		glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, (void*)0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		glUseProgram(0);
	}
private:
	void updateNormal(int idx1, int idx2, int idx3)
	{
		idx1 *= 8, idx2 *= 8, idx3 *= 8;

		float x1 = vertices[idx1 + 0], y1 = vertices[idx1 + 1], z1 = vertices[idx1 + 2];
		float x2 = vertices[idx2 + 0], y2 = vertices[idx2 + 1], z2 = vertices[idx2 + 2];
		float x3 = vertices[idx3 + 0], y3 = vertices[idx3 + 1], z3 = vertices[idx3 + 2];

		float v21x = x1 - x2;
		float v21y = y1 - y2;
		float v21z = z1 - z2;

		float v23x = x3 - x2;
		float v23y = y3 - y2;
		float v23z = z3 - z2;

		float nx = v23y * v21z - v23z * v21y;
		float ny = v23z * v21x - v23x * v21z;
		float nz = v23x * v21y - v23y * v21x;

		float length = sqrtf(nx * nx + ny * ny + nz * nz);

		nx /= length;
		ny /= length;
		nz /= length;

		vertices[idx2 + 3] = -nx, vertices[idx2 + 4] = -ny, vertices[idx2 + 5] = -nz;
	}
	void generateData()
	{
		vector<float> topCircle, baseCircle;
		for (int i = 0; i <= this->stackCount; ++i)
		{
			float y = (this->height / 2.0f) - (float)i / this->stackCount * this->height;
			float radius = this->topRadius + (float)i / this->stackCount * (this->baseRadius - this->topRadius);

			float ny = 0.0f;

			float v = 1 - (float)i / this->stackCount;

			for (int j = 0; j <= this->sectorCount; ++j)
			{
				float sectorAngle = (float)j / this->sectorCount * 2 * PI;

				float x = radius * cosf(sectorAngle);
				float z = radius * sinf(sectorAngle);

				float nx = cosf(sectorAngle);
				float nz = sinf(sectorAngle);

				float u = (float)j / this->sectorCount;

				vertices.insert(vertices.end(), { x, y, z, nx, ny, nz, u, v });

				if (i == 0)
				{
					float u = nx / 2.0f + 0.5f;
					float v = nz / 2.0f + 0.5f;

					topCircle.push_back(x), topCircle.push_back(y), topCircle.push_back(z);
					topCircle.push_back(0.0f), topCircle.push_back(1.0f), topCircle.push_back(0.0f);
					topCircle.push_back(u), topCircle.push_back(v);
				}
				else if (i == this->stackCount)
				{
					float u = nx / 2.0f + 0.5f;
					float v = nz / 2.0f + 0.5f;

					baseCircle.push_back(x), baseCircle.push_back(y), baseCircle.push_back(z);
					baseCircle.push_back(0.0f), baseCircle.push_back(-1.0f), baseCircle.push_back(0.0f);
					baseCircle.push_back(u), baseCircle.push_back(v);
				}
			}
		}

		for (auto a : topCircle)
		{
			vertices.push_back(a);
		}
		for (auto a : baseCircle)
		{
			vertices.push_back(a);
		}

		vertices.insert(vertices.end(), { 0.0f, this->height / 2.0f, 0.0f, 0.0f, 1.0f, 0.f, 0.5f, 0.5f });
		vertices.insert(vertices.end(), { 0.0f, -(this->height / 2.0f), 0.0f, 0.0f, -1.0f, 0.f, 0.5f, 0.5f });

		for (int i = 0; i < this->stackCount; ++i)
		{
			int k1 = i * (this->sectorCount + 1), k2 = k1 + (this->sectorCount + 1);
			for (int j = 0; j < this->sectorCount; ++j, ++k1, ++k2)
			{
				indices.push_back(k1), indices.push_back(k2), indices.push_back(k2 + 1);
				indices.push_back(k2 + 1), indices.push_back(k1 + 1), indices.push_back(k1);

				if (i != 0)
				{
					updateNormal(k1, k2, k2 + 1);
					updateNormal(k2 + 1, k1 + 1, k1);
				}
			}
		}

		textureCount0 = indices.size();

		// top Circle
		int k = (this->stackCount + 1) * (this->sectorCount + 1);
		int center = (this->stackCount + 3) * (this->sectorCount + 1);

		for (int i = 0; i < this->sectorCount; ++i, ++k)
		{
			indices.push_back(center), indices.push_back(k), indices.push_back(k + 1);
		}

		textureCount1 = indices.size();

		// base Circle
		k = (this->stackCount + 2) * (this->sectorCount + 1);
		center = center + 1;

		for (int i = 0; i < this->sectorCount; ++i, ++k)
		{
			indices.push_back(center), indices.push_back(k), indices.push_back(k + 1);
		}

		textureCount2 = indices.size();
	}
	void bindData()
	{
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(float),
			this->vertices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int),
			this->indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (int)(8 * sizeof(float)), (void*)(0 * sizeof(float)));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (int)(8 * sizeof(float)), (void*)(3 * sizeof(float)));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (int)(8 * sizeof(float)), (void*)(6 * sizeof(float)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	float topRadius, baseRadius, height;
	int stackCount, sectorCount;

	vec3 ambient, diffuse, specular;
	float shininess;

	unsigned int texture0, texture1, texture2;
	unsigned int textureCount0, textureCount1, textureCount2;

	unsigned int VAO, VBO, EBO;

	vector<float> coordinates;
	vector<float> normals;
	vector<float> textureCoordinates;

	vector<float> vertices;
	vector<unsigned int> indices;
};

#endif

