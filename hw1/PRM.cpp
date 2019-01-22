#include "stdafx.h"
#include "glad\glad.h"
#include "include\SDL.h"
#include "include\SDL_opengl.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <time.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <list>
#include <mmsystem.h>
#include <vector>
#include "PRM.h"
#include "objLoader.h"
using namespace std;

void sendRoadToBuffer(vector<glm::vec3> path, bufferID roadBufferID, GLuint shader) {
	GLfloat *points = new GLfloat[path.size() * 6];
	vector<glm::vec3>::iterator sn = path.begin();
	for (int i = 0; i < path.size(); i++, sn++) {
		points[i * 6] = sn->x;
		points[i * 6 + 1] = sn->y;
		points[i * 6 + 2] = sn->z;
		points[i * 6 + 3] = 1.0f;
		points[i * 6 + 4] = 0.9f;
		points[i * 6 + 5] = 0.3f;
	}

	glBindVertexArray(roadBufferID.vao);

	glBindBuffer(GL_ARRAY_BUFFER, roadBufferID.bufferID_vertex);
	glBufferData(GL_ARRAY_BUFFER, path.size() * 6 * sizeof(GLfloat), points, GL_STREAM_DRAW);

	int posAttrib = glGetAttribLocation(shader, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(posAttrib);

	int colAttrib = glGetAttribLocation(shader, "inColor");
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(colAttrib);
	glPointSize(10.0f);
	glDrawArrays(GL_POINTS, 0, path.size());
	glDrawArrays(GL_LINE_STRIP, 0, path.size());
	glBindVertexArray(0);
	delete points;
}

stateOnRoute computePositionOnRoute(vector<glm::vec3> path, stateOnRoute modelState) {
	GLfloat relativeAgentDistance = modelState.agentDistance;
	glm::vec3 relativeAgentPosition;
	GLfloat relativeAngle = 0;
	for (int i = 0; i < path.size(); i++) {
		if (i == (path.size() - 1)) {
			modelState.agentDistance = 0;
			modelState.relativeAgentPosition = relativeAgentPosition;
			modelState.relativeAngle = relativeAngle;
			return modelState;
		}
		glm::vec3 segment;
		segment = path[i + 1] - path[i];
		GLfloat segmentLength;
		segmentLength = sqrtf(glm::dot(segment, segment));
		if (segmentLength < relativeAgentDistance) {
			relativeAgentDistance -= segmentLength;
		}
		else {
			relativeAgentPosition = path[i] + (path[i + 1] - path[i]) * (relativeAgentDistance / segmentLength);
			relativeAngle = acosf(glm::dot(glm::vec3(0, 0, -1), segment) / segmentLength);
			if (segment.x > 0) {
				relativeAngle = 6.28f - relativeAngle;
			}
			modelState.relativeAgentPosition = relativeAgentPosition;
			modelState.relativeAngle = relativeAngle;
			return modelState;
		}
	}
}

velocityCone computeTruncatePoints(stateOnRoute from, stateOnRoute to) {
	GLfloat totalRadius;
	totalRadius = from.radius + to.radius;
	GLfloat x0, z0, x1, z1;
	x0 = from.position.x;
	z0 = from.position.z;
	x1 = to.position.x;
	z1 = to.position.z;
	GLfloat a, b, c;
	a = pow(x1 - x0, 2) - pow(totalRadius, 2);
	b = 2.0f * (x1 - x0) * (z0 - z1);
	c = pow(z0 - z1, 2) - pow(totalRadius, 2);
	GLfloat k1, k2;
	k1 = (-b + sqrtf(pow(b, 2) - 4.0f * a * c)) / 2.0f / a;
	k2 = (-b - sqrtf(pow(b, 2) - 4.0f * a * c)) / 2.0f / a;
	GLfloat tangentx1, tangenty1, tangentx2, tangenty2;
	a = 1 + pow(k1, 2);
	b = 2.0f * (-x1 + k1 * (z0 - k1 * x0 - z1));
	c = pow(x1, 2) + pow(z0 - k1 * x0 - z1, 2) - pow(totalRadius, 2);
	tangentx1 = -b / 2.0f / a;
	tangenty1 = k1 * tangentx1 + z0 - k1 * x0;
	a = 1 + pow(k2, 2);
	b = 2.0f * (-x1 + k2 * (z0 - k2 * x0 - z1));
	c = pow(x1, 2) + pow(z0 - k2 * x0 - z1, 2) - pow(totalRadius, 2);
	tangentx2 = -b / 2.0f / a;
	tangenty2 = k2 * tangentx2 + z0 - k2 * x0;

	/*glm::vec2 tan1(tangentx1 - x0, tangenty1 - z0);
	glm::vec2 tan2(tangentx2 - x0, tangenty2 - z0);
	GLfloat ratio;
	ratio = (sqrtf(pow(x1 - x0, 2) + pow(z1 - z0, 2)) - totalRadius) / sqrtf(pow(x1 - x0, 2) + pow(z1 - z0, 2));*/
	velocityCone cc;
	/*cc.truncatePoint1 = glm::vec2(tangentx1, tangenty1) + ratio * tan1;
	cc.truncatePoint2 = glm::vec2(tangentx2, tangenty2) + ratio * tan2;*/
	cc.truncatePoint1 = glm::vec2(tangentx1, tangenty1);
	cc.truncatePoint2 = glm::vec2(tangentx2, tangenty2);
	cc.apex = glm::vec2(0, 0);
	return cc;
}

void renderCone(velocityCone cone, GLuint shader, glm::vec3 color) {
	GLfloat particleAttributes[6 * 3];
	particleAttributes[0] = cone.truncatePoint1.x ;
	particleAttributes[1] = 0;
	particleAttributes[2] = cone.truncatePoint1.y ;
	particleAttributes[3] = color.x;
	particleAttributes[4] = color.y;
	particleAttributes[5] = color.z;
	particleAttributes[6] = cone.apex.x;
	particleAttributes[7] = 0;
	particleAttributes[8] = cone.apex.y;
	particleAttributes[9] = color.x;
	particleAttributes[10] = color.y;
	particleAttributes[11] = color.z;
	particleAttributes[12] = cone.truncatePoint2.x ;
	particleAttributes[13] = 0;
	particleAttributes[14] = cone.truncatePoint2.y ;
	particleAttributes[15] = color.x;
	particleAttributes[16] = color.y;
	particleAttributes[17] = color.z;

	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * 6 * sizeof(float), particleAttributes, GL_STREAM_DRAW);

	int posAttrib = glGetAttribLocation(shader, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(posAttrib);

	int colAttrib = glGetAttribLocation(shader, "inColor");
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(colAttrib);
	glUniform1i(glGetUniformLocation(shader, "texID"), -1);
	glm::mat4 transform;
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(transform));
	glPointSize(7.0f);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
}

bool checkIfInsideCone(velocityCone cone, glm::vec2 velocity, glm::vec2 vb) {
	glm::vec3 tan1, tan2;
	tan1 = glm::vec3(cone.truncatePoint1.x, 0, cone.truncatePoint1.y) - glm::vec3(cone.apex.x, 0, cone.apex.y);
	tan2 = glm::vec3(cone.truncatePoint2.x, 0, cone.truncatePoint2.y) - glm::vec3(cone.apex.x, 0, cone.apex.y);
	glm::vec3 translatedVelocity;
	translatedVelocity = glm::vec3((velocity - vb).x, 0, (velocity - vb).y);
	if (glm::dot(tan1 + tan2, translatedVelocity) < 0) {
		return FALSE;
	}
	glm::vec3 cross1, cross2;
	cross1 = glm::cross(translatedVelocity, tan1);
	cross2 = glm::cross(translatedVelocity, tan2);
	if (glm::dot(cross1, cross2) > 0) {
		//cout << "f" ;
		return FALSE;
	}
	else {
		//cout << "t" ;
		return TRUE;
	}
}

vector<glm::vec2> modifyVelocity(vector<glm::vec2> projectionVelocity, velocityCone cone, glm::vec2 vb) {
	//cout << "modi";
	vector<glm::vec2> newProjectionVelocity;
	for (int i = 0; i < projectionVelocity.size(); i++) {
		if (checkIfInsideCone(cone, projectionVelocity[i], vb)) {
			//cout << cone.truncatePoint1.x <<"   "<< cone.truncatePoint1.y<<"; "<< cone.truncatePoint2.x << "   " << cone.truncatePoint2.y << endl;
			//cout << "Not inside." << endl;
			glm::vec2 tan1, tan2;
			tan1 = glm::normalize(cone.truncatePoint1 - cone.apex);
			tan2 = glm::normalize(cone.truncatePoint2 - cone.apex);
			glm::vec2 projectionVelocity1, projectionVelocity2;
			projectionVelocity1 = tan1 * glm::dot(tan1, projectionVelocity[i] - vb);
			projectionVelocity2 = tan2 * glm::dot(tan2, projectionVelocity[i] - vb);
			
			glm::vec2 offset1, offset2;
			offset1 = projectionVelocity1 - projectionVelocity[i];
			offset2 = projectionVelocity2 - projectionVelocity[i];
			offset1 *= 1.001f;
			offset2 *= 1.001f;
			projectionVelocity1 = offset1 + projectionVelocity[i];
			projectionVelocity2 = offset2 + projectionVelocity[i];

			projectionVelocity1 += vb;
			projectionVelocity2 += vb;
			newProjectionVelocity.push_back(projectionVelocity1);
			newProjectionVelocity.push_back(projectionVelocity2);
		}
		else {
			newProjectionVelocity.push_back(projectionVelocity[i]);
		}
	}
	return newProjectionVelocity;
}

vector<glm::vec2> removeUnqualifiedVelocity(vector<glm::vec2> projectionVelocity, vector<pair<velocityCone, glm::vec2>> pastCone) {
	//cout << "remo";
	for (int i = 0; i < pastCone.size(); i++) {
		velocityCone cone= pastCone[i].first;
		glm::vec2 vb = pastCone[i].second;
		for (int j = 0; j < projectionVelocity.size(); j++) {
			if (checkIfInsideCone(cone, projectionVelocity[j], vb)) {
				projectionVelocity.erase(projectionVelocity.begin() + j);
			}
		}
	}
	return projectionVelocity;
}

vector<stateOnRoute> RVO(vector<stateOnRoute> obj, GLfloat dt, vector<stateOnRoute> obstacles, GLuint shader) {
	
	vector<stateOnRoute> objectState = obj;
	
	for (int i = 0; i < objectState.size(); i++) {
		objectState[i].PRM.erase(objectState[i].PRM.begin());
		if (glm::distance(objectState[i].position, objectState[i].PRM[0]) < 0.001f) {
			if (objectState[i].PRM.size() > 1) {
				objectState[i].PRM.erase(objectState[i].PRM.begin());
			}
		}
		objectState[i].velocity = objectState[i].agentSpeed * glm::normalize(objectState[i].PRM[0] - objectState[i].position);
		glm::vec2 twoDspeed(objectState[i].velocity.x, objectState[i].velocity.z);
		bool checkObstacles = false;
		vector<glm::vec2> projectionVelocity;
		projectionVelocity.push_back(twoDspeed);
		vector<pair<velocityCone, glm::vec2>> pastCone;
		for (int j = 0; j < obstacles.size(); j++) {
			glm::vec2 vb;
			//vb = glm::vec2(obstacles[j].velocity.x, obstacles[j].velocity.z);
			vb = glm::vec2(0, 0);
			velocityCone cone;
			cone = computeTruncatePoints(obj[i], obstacles[j]);
			cone.apex = glm::vec2(obj[i].position.x, obj[i].position.z);
			//if (j == 0) {
			//	cout << endl;
			//	cout << "0: ";
			////renderCone(cone, shader, glm::vec3(1.0f, 0, 0));
			//}
			//if (j == 1) {
			//	cout << endl;
			//	cout << "1: ";
			//	cout <<"p1:"<< cone.truncatePoint1.x << ' ' << cone.truncatePoint1.y ;
			//	cout << "p2:" << cone.truncatePoint2.x << ' ' << cone.truncatePoint2.y << endl;
			//renderCone(cone, shader, glm::vec3(0, 1.0f, 0));
			//}
			//if (j == 2) {
			//	cout << endl;
			//	cout << "2: ";
			////renderCone(cone, shader, glm::vec3(0, 0, 1.0f));
			//}
			projectionVelocity = modifyVelocity(projectionVelocity, cone, vb);
			projectionVelocity = removeUnqualifiedVelocity(projectionVelocity, pastCone);
			pastCone.push_back(make_pair(cone, vb));
		}
		for (int j = 0; j < obj.size(); j++) {
			if (i != j) {
				glm::vec2 vb;
				vb = glm::vec2(obj[j].velocity.x, obj[j].velocity.z);
				velocityCone cone;
				cone = computeTruncatePoints(obj[i], obj[j]);
				cone.apex = glm::vec2(obj[i].position.x, obj[i].position.z);
				//renderCone(cone, shader, glm::vec3(1.0f, 1.0f, 0));
				projectionVelocity = modifyVelocity(projectionVelocity, cone, vb);
				projectionVelocity = removeUnqualifiedVelocity(projectionVelocity, pastCone);
				pastCone.push_back(make_pair(cone, vb));
			}
		}
		if (projectionVelocity.size() != 0) {
			GLfloat min = 9999.0f;
			glm::vec2 closetNode;
			for (int m = 0; m < projectionVelocity.size(); m++) {
				GLfloat distance = sqrtf(glm::dot(projectionVelocity[m] - twoDspeed, projectionVelocity[m] - twoDspeed));
				if (distance < min) {
					min = distance;
					closetNode = projectionVelocity[m];
				}
			}
			objectState[i].velocity = glm::vec3(closetNode.x, 0, closetNode.y);
		}
		//cout << objectState[i].velocity.x << ' ' << objectState[i].velocity.z << endl;
		//velocityCone cone;
		//cone.truncatePoint1 = glm::vec2(objectState[i].velocity.x, objectState[i].velocity.z);
		//cone.truncatePoint2 = glm::vec2(objectState[i].velocity.x, objectState[i].velocity.z);
		//cone.apex= glm::vec2(objectState[i].position.x, objectState[i].position.z);
		////cout << objectState[i].position.x << ' ' << objectState[i].position.z << endl;
		//renderCone(cone, shader, glm::vec3(1.0f, 1.0f, 1.0f));
		////[i].position += dt * obj[i].velocity;
	}
	return objectState;
}

boid* boidMovement(boid* boids, GLfloat dt, GLfloat boidRouteRadius, GLfloat boidNumber, GLfloat separationRadius, GLfloat alignmentRadius, GLfloat cohesionRadius) {
	for (int i = 0; i < boidNumber; i++) {

		vector<pair<int, GLfloat>> neighbor;
		for (int j = 0; j < boidNumber; j++) {
			if (j != i) {
				GLfloat distance;
				distance = sqrtf(glm::dot(boids[j].position - boids[i].position, boids[j].position - boids[i].position));
				neighbor.push_back(make_pair(boids[j].label, distance));
			}
		}
		for (int j = 0; j < neighbor.size(); j++) {
			for (int k = 0; k < neighbor.size() - 1; k++) {
				if (neighbor[k].second > neighbor[k + 1].second) {
					swap(neighbor[k], neighbor[k + 1]);
				}
			}
		}
		// Separation Force
		glm::vec3 separationForce(0, 0, 0);
		int count = 0;
		for (int j = 0; j < neighbor.size(); j++) {
			if (neighbor[j].second > 0 && neighbor[j].second < separationRadius) {
				separationForce += glm::normalize(boids[i].position - boids[neighbor[j].first].position) / neighbor[j].second;
				count++;
			}
		}
		if (count > 0){
			separationForce /= (float)count;
			separationForce = glm::normalize(separationForce) * boids[i].maxSpeed;
			separationForce -= boids[i].velocity;
			float magnitude = sqrt(glm::dot(separationForce, separationForce));
			if (magnitude > boids[i].maxForce) {
				separationForce *= (boids[i].maxForce / magnitude);
			}
		}

		// Alignment Force
		count = 0;
		glm::vec3 alignmentForce(0, 0, 0);
		for (int j = 0; j < neighbor.size(); j++) {
			if (neighbor[j].second > 0 && neighbor[j].second < alignmentRadius) {
				alignmentForce += boids[neighbor[j].first].velocity;
				count++;
			}
		}
		if (count > 0) {
			alignmentForce /= (float)count;
			alignmentForce = glm::normalize(alignmentForce) * boids[i].maxSpeed;
			alignmentForce -= boids[i].velocity;
			float magnitude = sqrt(glm::dot(alignmentForce, alignmentForce));
			if (magnitude > boids[i].maxForce) {
				alignmentForce *= (boids[i].maxForce / magnitude);
			}
		}

		// Cohesion Force
		count = 0;
		glm::vec3 cohesionForce(0, 0, 0);
		for (int j = 0; j < neighbor.size(); j++) {
			if (neighbor[j].second > 0 && neighbor[j].second < cohesionRadius) {
				cohesionForce += boids[neighbor[j].first].position;
				count++;
			}
		}
		if (count > 0) {
			cohesionForce /= (float)count;
		}

		boids[i].acceleration += 1.5f * separationForce + 3.0f * alignmentForce + 1.5f * cohesionForce;
		boids[i].velocity += dt * boids[i].acceleration;
		float magnitude = sqrt(glm::dot(boids[i].velocity, boids[i].velocity));
		if (magnitude > boids[i].maxSpeed) {
			boids[i].velocity *= (boids[i].maxSpeed / magnitude);
		}
		boids[i].position += dt * boids[i].velocity;
		boids[i].acceleration = glm::vec3(0, 0, 0);

		GLfloat distanceToCenter;
		distanceToCenter = sqrtf(glm::dot(boids[i].position - glm::vec3(0, 0, 0), boids[i].position - glm::vec3(0, 0, 0)));
		if (distanceToCenter > boidRouteRadius) {
			GLfloat offsetx, offsety, offsetz;
			offsetx = (rand() % 100) / 100.0f;
			offsety = (rand() % 100) / 100.0f;
			offsetz = (rand() % 100) / 100.0f;
			GLfloat angle1, angle2;
			angle1 = (rand() % 628) / 100.0f;
			angle2 = (rand() % 628) / 100.0f;
			GLfloat x, y, z;
			y = boidRouteRadius * sinf(angle1) * offsetx;
			x = abs(y) * cosf(angle2);
			z = abs(y) * sinf(angle2);
			boids[i].position = glm::vec3(x, y, z);
			boids[i].velocity = glm::normalize(glm::vec3(offsetx - 0.5f, offsety - 0.5f, offsetz - 0.5f)) * boids[i].maxSpeed;
		}
	}
	return boids;
}

void renderBoids(boid* boids, float* boidsTriangle, GLfloat boidNumber, GLuint shader, GLuint* boidsIndex) {
	for (int i = 0; i < boidNumber; i++) {
		glBindVertexArray(boids[i].buffer.vao);

		glBindBuffer(GL_ARRAY_BUFFER, boids[i].buffer.bufferID_vertex);
		glBufferData(GL_ARRAY_BUFFER, 5 * 9 * sizeof(float), boidsTriangle, GL_STREAM_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boids[i].buffer.bufferID_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * 3 * sizeof(unsigned int), boidsIndex, GL_STREAM_DRAW);

		int posAttrib = glGetAttribLocation(shader, "position");
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
		glEnableVertexAttribArray(posAttrib);

		int colAttrib = glGetAttribLocation(shader, "inColor");
		glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(colAttrib);

		int normAttrib = glGetAttribLocation(shader, "inNormal");
		glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(normAttrib);

		GLfloat relativeAngle2 = acos(glm::dot(glm::normalize(glm::vec3(0, 1.0f, 0)), glm::normalize(boids[i].velocity)));
		glm::mat4 transform;
		transform = glm::translate(transform, boids[i].position);
		transform = glm::rotate(transform, relativeAngle2, glm::cross(glm::vec3(0, 1.0f, 0), boids[i].velocity));
		transform = glm::scale(transform, glm::vec3(0.5f, 1.0f, 0.5f));
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(transform));
		glUniform1i(glGetUniformLocation(shader, "texID"), -1);
		glUniform1f(glGetUniformLocation(shader, "dr"), 0.4f);
		glUniform1f(glGetUniformLocation(shader, "dg"), 0.1f);
		glUniform1f(glGetUniformLocation(shader, "db"), 0.1f);
		glUniform1f(glGetUniformLocation(shader, "ar"), 0.7f);
		glUniform1f(glGetUniformLocation(shader, "ag"), 0.3f);
		glUniform1f(glGetUniformLocation(shader, "ab"), 0.7f);
		glUniform1f(glGetUniformLocation(shader, "sr"), 0.01f);
		glUniform1f(glGetUniformLocation(shader, "sg"), 0.01f);
		glUniform1f(glGetUniformLocation(shader, "sb"), 0.01f);
		glUniform1f(glGetUniformLocation(shader, "ns"), 96.0f);

		glDrawElements(GL_TRIANGLES, 3 * 6, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
	}
}

void renderCircleBeneathModel(vector<stateOnRoute> obj, vector<stateOnRoute> obstacles, GLuint shader, GLuint* circleIndex, bufferID circleBeneathModelBuffer) {
	GLfloat circle[13 * 6];
	for (int i = 0; i < obj.size(); i++) {
		for (int j = 1; j < 13; j++) {
			GLfloat arc = (j - 1) / 12.0f * 6.28f;
			GLfloat x, z;
			x = obj[i].radius * cosf(arc);
			z = obj[i].radius * sinf(arc);
			circle[j * 6] = x + obj[i].position.x;
			circle[j * 6 + 1] = 0;
			circle[j * 6 + 2] = z + obj[i].position.z;
			circle[j * 6 + 3] = 0.910f;
			circle[j * 6 + 4] = 0.186f;
			circle[j * 6 + 5] = 1.0f;
		}
		circle[0] = obj[i].position.x;
		circle[1] = 0;
		circle[2] = obj[i].position.z;
		circle[3] = 0.910f;
		circle[4] = 0.186f;
		circle[5] = 1.0f;

		glBindVertexArray(circleBeneathModelBuffer.vao);

		glBindBuffer(GL_ARRAY_BUFFER, circleBeneathModelBuffer.bufferID_vertex);
		glBufferData(GL_ARRAY_BUFFER, 13 * 6 * sizeof(float), circle, GL_STREAM_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleBeneathModelBuffer.bufferID_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * 3 * sizeof(unsigned int), circleIndex, GL_STREAM_DRAW);

		int posAttrib = glGetAttribLocation(shader, "position");
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
		glEnableVertexAttribArray(posAttrib);

		int colAttrib = glGetAttribLocation(shader, "inColor");
		glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(colAttrib);

		glm::mat4 transform;
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(transform));
		glUniform1i(glGetUniformLocation(shader, "texID"), -1);
		glDrawElements(GL_TRIANGLES, 3 * 12, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
	}

	for (int i = 0; i < obstacles.size(); i++) {
		for (int j = 1; j < 13; j++) {
			GLfloat arc = (j - 1) / 12.0f * 6.28f;
			GLfloat x, z;
			x = obstacles[i].radius * cosf(arc);
			z = obstacles[i].radius * sinf(arc);
			circle[j * 6] = x + obstacles[i].position.x;
			circle[j * 6 + 1] = 0;
			circle[j * 6 + 2] = z + obstacles[i].position.z;
			circle[j * 6 + 3] = 1.0f;
			circle[j * 6 + 4] = 0.9f;
			circle[j * 6 + 5] = 0.2f;
		}
		circle[0] = obstacles[i].position.x;
		circle[1] = 0;
		circle[2] = obstacles[i].position.z;
		circle[3] = 1.0f;
		circle[4] = 0.9f;
		circle[5] = 0.2f;

		glBindVertexArray(circleBeneathModelBuffer.vao);

		glBindBuffer(GL_ARRAY_BUFFER, circleBeneathModelBuffer.bufferID_vertex);
		glBufferData(GL_ARRAY_BUFFER, 13 * 6 * sizeof(float), circle, GL_STREAM_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleBeneathModelBuffer.bufferID_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * 3 * sizeof(unsigned int), circleIndex, GL_STREAM_DRAW);

		int posAttrib = glGetAttribLocation(shader, "position");
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
		glEnableVertexAttribArray(posAttrib);

		int colAttrib = glGetAttribLocation(shader, "inColor");
		glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(colAttrib);

		glm::mat4 transform;
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(transform));
		glUniform1i(glGetUniformLocation(shader, "texID"), -1);
		glDrawElements(GL_TRIANGLES, 3 * 12, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
	}
}

bool samplingPointInsideObstacles(glm::vec3 point, vector<stateOnRoute> obstacles, stateOnRoute agent) {
	for (int i = 0; i < obstacles.size(); i++) {
		GLfloat distance;
		distance = glm::distance(point, obstacles[i].position);
		if (distance < (obstacles[i].radius +  agent.radius)) {
			return TRUE;
		}
	}
	return FALSE;
}

bool samplingPointIntersection(glm::vec3 point1, glm::vec3 point2, vector<stateOnRoute> obstacles, stateOnRoute agent) {
	for (int i = 0; i < obstacles.size(); i++) {
		glm::vec3 line = glm::normalize(point2 - point1);
		glm::vec3 pointToCenter = obstacles[i].position - point1;
		GLfloat projection = glm::dot(line, pointToCenter);
		GLfloat distance = sqrt(glm::dot(pointToCenter, pointToCenter) - pow(projection, 2));
		if (distance < (obstacles[i].radius + agent.radius)) {
			return TRUE;
		}
	}
	return FALSE;
}

vector<glm::vec3> PRM(stateOnRoute agent, vector<stateOnRoute> obstacles, GLuint sampleNumber) {
	// Randomly sample configurations
	vector<glm::vec3> sampleNodes;
	for (int i = 0; i < sampleNumber; i++) {
		GLfloat tempx, tempz;
		tempx = (rand() % 800) / 100.0f - 4.0f;
		tempz = (rand() % 800) / 100.0f - 4.0f;
		if (!samplingPointInsideObstacles(glm::vec3(tempx, 0, tempz), obstacles, agent)) {
			sampleNodes.push_back(glm::vec3(tempx, 0, tempz));
		}
	}
	sampleNodes.push_back(agent.position);
	sampleNodes.push_back(agent.goal);

	// Erase isolated points
	for (int i = 0; i < sampleNodes.size(); i++) {
		bool isolated = TRUE;
		for (int j = 0; j < sampleNodes.size(); j++) {
			if (i != j) {
				if (!samplingPointIntersection(sampleNodes[i], sampleNodes[j], obstacles, agent)) {
					isolated = FALSE;
				}
			}
		}
		if (isolated) {
			sampleNodes.erase(sampleNodes.begin() + i);
			i--;
		}
	}

	// Generate adjacency matrix
	GLfloat **adjMatrix = new GLfloat*[sampleNodes.size()];
	for (int i = 0; i < sampleNodes.size(); i++) {
		adjMatrix[i] = new GLfloat[sampleNodes.size()];
	}
	for (int i = 0; i < sampleNodes.size(); i++) {
		for (int j = 0; j < sampleNodes.size(); j++) {
			if (i != j) {
				if (samplingPointIntersection(sampleNodes[i], sampleNodes[j], obstacles, agent)) {
					adjMatrix[i][j] = 9999.0f;
				}
				else {
					adjMatrix[i][j] = sqrtf(glm::dot(sampleNodes[i] - sampleNodes[j], sampleNodes[i] - sampleNodes[j]));
				}
			}
			else {
				adjMatrix[i][j] = 9999.0f;
			}
			cout << adjMatrix[i][j] <<"   ";
		}
		cout << endl;
	}

	// Find the shortestpath
	int *shortestPath = new int[sampleNodes.size()];
	bool *finalPath = new bool[sampleNodes.size()];
	GLfloat *weight = new GLfloat[sampleNodes.size()];
	for (int i = 0; i < sampleNodes.size(); i++) {
		finalPath[i] = FALSE;
		weight[i] = adjMatrix[sampleNodes.size() - 2][i];
		if (weight[i] > 0) {
			shortestPath[i] = sampleNodes.size() - 2;
		}

	}
	finalPath[sampleNodes.size() - 2] = TRUE;
	for (int i = 1; i < sampleNodes.size(); i++) {
		GLfloat Min = 9999.0f;
		int minPosition;
		for (int j = 0; j < sampleNodes.size(); j++) {
			cout << weight[j] << "  ";
			if (!finalPath[j] && (weight[j] < Min)) {
				Min = weight[j];
				minPosition = j;
			}
		}
		//cout << endl;
		finalPath[minPosition] = TRUE;
		for (int j = 0; j < sampleNodes.size(); j++) {
			if (!finalPath[j] && (adjMatrix[minPosition][j] + Min) < weight[j]) {
				weight[j] = adjMatrix[minPosition][j] + Min;
				shortestPath[j] = minPosition;
			}
		}
	}
	vector<glm::vec3> path2;
	path2.push_back(sampleNodes[sampleNodes.size() - 1]);
	int lastNode = shortestPath[sampleNodes.size() - 1];
	while (lastNode != (sampleNodes.size() - 2)) {
		path2.push_back(sampleNodes[lastNode]);
		lastNode = shortestPath[lastNode];
	}
	path2.push_back(sampleNodes[sampleNodes.size() - 2]);
	// Turn the vector upside down
	vector<glm::vec3> path;
	for (int i = 0; i < path2.size(); i++) {
		path.push_back(path2[path2.size() - 1 - i]);
	}

	return path;
}
