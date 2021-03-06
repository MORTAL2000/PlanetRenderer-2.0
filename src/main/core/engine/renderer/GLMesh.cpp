#include "GLMesh.h"
#include "core/application/Application.h"
#include <GL/glew.h>



InstanceBuffer::InstanceBuffer(int32 instanceSizeBytes, int32 instanceCount, int32 divisor, std::vector<InstanceAttribute> attributes):
	instanceSizeBytes(instanceSizeBytes), instanceCount(instanceCount), divisor(divisor), attributes(attributes) {

	glGenBuffers(1, &this->instanceBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, this->instanceBuffer);
	glBufferData(GL_ARRAY_BUFFER, instanceSizeBytes * instanceCount, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InstanceBuffer::uploadInstanceData(uint32 offset, uint32 size, void* data) {
	glBindBuffer(GL_ARRAY_BUFFER, this->instanceBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InstanceBuffer::bind(bool bind) {
	if (bind) {
		this->enableAttributes(true);
		glBindBuffer(GL_ARRAY_BUFFER, this->instanceBuffer);

		for (int i = 0; i < this->attributes.size(); i++) {
			InstanceAttribute attr = this->attributes[i];
			glEnableVertexAttribArray(attr.index);
			if (attr.type == GL_FLOAT || attr.type == GL_DOUBLE)
				glVertexAttribPointer(attr.index, attr.size, attr.type, GL_FALSE, this->instanceSizeBytes, BUFFER_OFFSET(attr.offset));
			else 
				glVertexAttribIPointer(attr.index, attr.size, attr.type, this->instanceSizeBytes, BUFFER_OFFSET(attr.offset));
			glVertexAttribDivisor(attr.index, this->divisor);
		}

	} else {
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		for (int i = 0; i < this->attributes.size(); i++) {
			InstanceAttribute attr = this->attributes[i];
			glDisableVertexAttribArray(attr.index);
		}
	}
}

void InstanceBuffer::enableAttributes(bool enabled) {
	for (int i = 0; i < this->attributes.size(); i++) {
		InstanceAttribute attr = this->attributes[i];
		if (enabled) {
			glEnableVertexAttribArray(attr.index);
		} else {
			glDisableVertexAttribArray(attr.index);
		}
	}
}


GLMesh::GLMesh(MeshData* meshData, VertexLayout attributes) :
	attributes(attributes), vertexCount(0), indexCount(0), allocatedVertexBufferSize(1), allocatedIndexBufferSize(1) {

	glGenVertexArrays(1, &this->vertexArray);
	glBindVertexArray(this->vertexArray);

	glGenBuffers(1, &this->vertexBuffer);
	glGenBuffers(1, &this->indexBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
	bindVertexAttribLayout(this->attributes);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (meshData != NULL) {
		this->reserveBuffers(meshData->getVertexBufferSize(), meshData->getIndexBufferSize());
		this->uploadMeshData(meshData);
	} else {
		this->reserveBuffers(1, 1);
	}
	
	glBindVertexArray(0);

	this->setPrimitive(GL_TRIANGLES);
}

GLMesh::~GLMesh() {
	glDeleteBuffers(1, &this->vertexBuffer);
	glDeleteBuffers(1, &this->indexBuffer);
	glDeleteVertexArrays(1, &this->vertexArray);
}

void GLMesh::uploadMeshData(MeshData* meshData) {
	this->vertexCount = meshData->getVertexCount();
	this->indexCount = meshData->getIndexCount();

	if (meshData->getVertexBufferSize() > this->allocatedVertexBufferSize || meshData->getIndexBufferSize() > this->allocatedIndexBufferSize) {
		this->reserveBuffers(meshData->getVertexBufferSize(), meshData->getIndexBufferSize());
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBuffer);

	glBufferSubData(GL_ARRAY_BUFFER, 0, meshData->getVertexBufferSize(), meshData->getVertexBufferData());
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, meshData->getIndexBufferSize(), meshData->getIndexBufferData());

	//glBufferData(GL_ARRAY_BUFFER, meshData->getVertexBufferSize(), meshData->getVertexBufferData(), GL_DYNAMIC_DRAW);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData->getIndexBufferSize(), meshData->getIndexBufferData(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLMesh::reserveBuffers(int32 vertexBufferSize, int32 indexBufferSize) {

	logInfo("Reserving %d vertex bytes and %d index bytes", vertexBufferSize, indexBufferSize);
	this->allocatedVertexBufferSize = vertexBufferSize;
	this->allocatedIndexBufferSize = indexBufferSize;

	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, this->allocatedVertexBufferSize, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->allocatedIndexBufferSize, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void GLMesh::draw(int32 instances, int32 offset, int32 count, InstanceBuffer* instanceBuffer) {

	//glBindVertexArray(this->vertexArray);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBuffer);
	//
	//enableVertexAttribLayout(this->attributes, true);
	//glDrawElements(GL_TRIANGLES, this->indexCount, GL_UNSIGNED_INT, NULL);
	//enableVertexAttribLayout(this->attributes, false);
	//
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);

	glBindVertexArray(vertexArray);
	enableVertexAttribLayout(this->attributes, true);
	
	if (instances > 0) {                // If there are zero instances we do nothing...
		if (this->vertexCount > 0) {    // If there are zero vertices we do nothing...
			if (offset > 0)             // If the index offset is not zero, make sure that "offset + count" does not exceed "indexCount"
				count = glm::min(offset + (count > 0 ? count : this->indexCount), this->indexCount) - offset;
	
			if (instanceBuffer != NULL)
				instanceBuffer->bind(true);

			if (this->indexCount > 0) { // If there is an index buffer, we want to draw elements
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBuffer);
				if (instances == 1) {   // If there is only one instance to draw
					if (count > 0)      // draw 1 instance of the indices between "offset" and "offset + count" in the index array
						glDrawElements(this->primitive, count, GL_UNSIGNED_INT, (void*)offset);
					else                // draw 1 instance of the whole index buffer
						glDrawElements(this->primitive, this->indexCount, GL_UNSIGNED_INT, (void*)0);
				} else {                // If there are multiple instances to draw
					if (count > 0)      // draw "instances" instances of the indices between "offset" and "offset + count" in the index array
						glDrawElementsInstanced(this->primitive, count, GL_UNSIGNED_INT, (void*)offset, instances);
					else                // draw "instances" instances of the whole index buffer
						glDrawElementsInstanced(this->primitive, this->indexCount, GL_UNSIGNED_INT, (void*)0, instances);
				}
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			} else {                    // If there is no index buffer we want to draw arrays
				if (instances == 1) {   // If there is only one instance to draw
					if (count > 0)      // draw 1 instance of the vertices between "offset" and "offset + count" in the vertex array
						glDrawArrays(this->primitive, offset, count);
					else                // draw 1 instance of the whole vertex buffer
						glDrawArrays(this->primitive, 0, this->vertexCount);
				} else {                // If there are multiple instances to draw
					if (count > 0)      // draw "instances" instances of the vertices between "offset" and "offset + count" in the vertex array
						glDrawArraysInstanced(this->primitive, offset, count, instances);
					else                // draw "instances" instances of the whole vertices buffer
						glDrawArraysInstanced(this->primitive, 0, this->vertexCount, instances);
				}
			}

			//if (instanceBuffer != NULL) {
			//	instanceBuffer->bind(false);
			//}
		}
	}
	
	enableVertexAttribLayout(this->attributes, false);
	glBindVertexArray(0);
}

void GLMesh::setPrimitive(uint32 primitive) {
	this->primitive = primitive;
}

uint32 GLMesh::getVertexArray() {
	return this->vertexArray;
}

uint32 GLMesh::getVertexBuffer() {
	return this->vertexBuffer;
}

uint32 GLMesh::getIndexBuffer() {
	return this->indexBuffer;
}

uint32 GLMesh::getPrimitive() const {
	return this->primitive;
}

void GLMesh::bindVertexAttribLayout(VertexLayout layout) {
	for (int i = 0; i < layout.attributes.size(); i++) {
		VertexAttribute attrib = layout.attributes[i];
		glVertexAttribPointer(attrib.index, attrib.size, GL_FLOAT, GL_FALSE, layout.stride, BUFFER_OFFSET(attrib.offset));
	}
}

void GLMesh::enableVertexAttribLayout(VertexLayout layout, bool enabled) {
	for (int i = 0; i < layout.attributes.size(); i++) {
		VertexAttribute attrib = layout.attributes[i];
		if (enabled) {
			glEnableVertexAttribArray(attrib.index);
		} else {
			glDisableVertexAttribArray(attrib.index);
		}
	}
}
