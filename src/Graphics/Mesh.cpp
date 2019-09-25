/* openblack - A reimplementation of Lionhead's Black & White.
 *
 * openblack is the legal property of its developers, whose names
 * can be found in the AUTHORS.md file distributed with this source
 * distribution.
 *
 * openblack is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * openblack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with openblack. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Mesh.h"

#include <stdexcept>
#include <array>

#include "ShaderProgram.h"

#include "OpenGL.h"

using namespace openblack::graphics;

namespace
{
constexpr std::array<GLenum, 5> topologies = {
	GL_POINTS,
	GL_LINES,
	GL_LINE_STRIP,
	GL_TRIANGLES,
	GL_TRIANGLE_STRIP,
};
}

Mesh::Mesh(VertexBuffer* vertexBuffer, Topology topology)
	: _vertexBuffer(vertexBuffer)
	, _topology(topology)
{
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	_vertexBuffer->Bind();
	_vertexBuffer->bindVertexDecl();

	glBindVertexArray(0);
}

Mesh::Mesh(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, Topology topology)
	: _vertexBuffer(vertexBuffer)
	, _indexBuffer(indexBuffer)
	, _topology(topology)
{
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	_vertexBuffer->Bind();
	_vertexBuffer->bindVertexDecl();
	_indexBuffer->Bind(_indexBuffer->GetCount());

	glBindVertexArray(0);
}

Mesh::~Mesh()
{
	if (_vao != 0)
		glDeleteVertexArrays(1, &_vao);
}

const VertexBuffer& Mesh::GetVertexBuffer() const
{
	return *_vertexBuffer;
}

const IndexBuffer& Mesh::GetIndexBuffer() const
{
	return *_indexBuffer;
}

Mesh::Topology Mesh::GetTopology() const noexcept
{
	return _topology;
}

void Mesh::Draw(uint8_t viewId, const openblack::graphics::ShaderProgram &program, uint64_t state, uint32_t rgba)
{
	if (_indexBuffer != nullptr && _indexBuffer->GetCount() > 0)
	{
		Draw(viewId, program, _indexBuffer->GetCount(), 0, state, rgba);
	}
	else
	{
		Draw(viewId, program, _vertexBuffer->GetVertexCount(), 0, state, rgba);
	}
}

void Mesh::Draw(uint8_t viewId, const openblack::graphics::ShaderProgram &program, uint32_t count, uint32_t startIndex, uint64_t state = 0, uint32_t rgba = 0)
{
	auto topology = topologies[static_cast<size_t>(_topology)];
	if (_indexBuffer != nullptr && _indexBuffer->GetCount() > 0)
	{
		_indexBuffer->Bind(count, startIndex);
	}
	_vertexBuffer->Bind();

	// Set render states.
	bgfx::setState(state);

	bgfx::submit(viewId, program.GetRawHandle());
}
