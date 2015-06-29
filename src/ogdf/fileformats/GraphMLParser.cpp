/** \file
 * \brief Implementation of GraphML parser.
 *
 * \author Łukasz Hanuszczak
 *
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 *
 * \par
 * Copyright (C)<br>
 * See README.txt in the root directory of the OGDF installation for details.
 *
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation;
 * see the file LICENSE.txt included in the packaging of this file
 * for details.
 *
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * \par
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/

#include <ogdf/fileformats/GraphMLParser.h>
#include <ogdf/fileformats/GraphML.h>

namespace ogdf {


GraphMLParser::GraphMLParser(istream &in) : m_xml(in)
{
	m_error = false;
	if (m_xml.createParseTree() == false) {
		OGDF_ERROR("Parse error.");
		m_error = true;
		return;
	}

	const XmlTagObject &rootTag = m_xml.getRootTag();
	if(rootTag.getName() != "graphml") {
		OGDF_ERROR("File root tag is not a <graphml>.");
		m_error = true;
		return;
	}

	rootTag.findSonXmlTagObjectByName("graph", m_graphTag);
	if (m_graphTag == nullptr) {
		OGDF_ERROR("<graph> tag not found.");
		m_error = true;
		return;
	}

	List<XmlTagObject *> keyTags;
	rootTag.findSonXmlTagObjectByName("key", keyTags);

	for(XmlTagObject *obj : keyTags) {
		const XmlTagObject &keyTag = *obj;

		XmlAttributeObject *idAttr, *nameAttr;
		keyTag.findXmlAttributeObjectByName("id", idAttr);
		keyTag.findXmlAttributeObjectByName("attr.name", nameAttr);

		if (idAttr == nullptr) {
			OGDF_ERROR("Key does not have an id attribute.");
			m_error = true;
			return;
		}
		if (nameAttr == nullptr) {
			OGDF_ERROR("Key does not have an attr.name attribute.");
			m_error = true;
			return;
		}

		m_attrName[idAttr->getValue()] = nameAttr->getValue();
	}
}


GraphMLParser::~GraphMLParser()
{
}


bool GraphMLParser::readData(
	GraphAttributes &GA,
	const node &v, const XmlTagObject &nodeData)
{
	XmlAttributeObject *keyId;
	nodeData.findXmlAttributeObjectByName("key", keyId);

	if (keyId == nullptr) {
		OGDF_ERROR("Node data does not have a key.");
		return false;
	}

	const long attrs = GA.attributes();
	std::stringstream value(nodeData.getValue());

	switch (graphml::toAttribute(m_attrName[keyId->getValue()])) {
	case graphml::a_nodeLabel:
		if(attrs & GraphAttributes::nodeLabel) {
			value >> GA.label(v);
		}
		break;
	case graphml::a_x:
		if(attrs & GraphAttributes::nodeGraphics) {
			value >> GA.x(v);
		}
		break;
	case graphml::a_y:
		if(attrs & GraphAttributes::nodeGraphics) {
			value >> GA.y(v);
		}
		break;
	case graphml::a_width:
		if(attrs & GraphAttributes::nodeGraphics) {
			value >> GA.width(v);
		}
		break;
	case graphml::a_height:
		if(attrs & GraphAttributes::nodeGraphics) {
			value >> GA.height(v);
		}
		break;
	case graphml::a_size:
		if(attrs & GraphAttributes::nodeGraphics) {
			double size;
			value >> size;
			// We want to set a new size only if width and height was not set.
			if (GA.height(v) == GA.width(v)) {
				GA.height(v) = GA.width(v) = size;
			}
		}
		break;
	case graphml::a_shape:
		if(attrs & GraphAttributes::nodeGraphics) {
			std::string str;
			value >> str;
			GA.shape(v) = graphml::toShape(str);
		}
		break;
	case graphml::a_z:
		if(attrs & GraphAttributes::threeD) {
			value >> GA.z(v);
		}
		break;
	case graphml::a_r:
		if(attrs & GraphAttributes::nodeStyle) {
			int r;
			value >> r;
			GA.fillColor(v).red(static_cast<uint8_t>(r));
		}
		break;
	case graphml::a_g:
		if(attrs & GraphAttributes::nodeStyle) {
			int g;
			value >> g;
			GA.fillColor(v).green(static_cast<uint8_t>(g));
		}
		break;
	case graphml::a_b:
		if(attrs & GraphAttributes::nodeStyle) {
			int b;
			value >> b;
			GA.fillColor(v).blue(static_cast<uint8_t>(b));
		}
		break;
	case graphml::a_nodeFill:
		if(attrs & GraphAttributes::nodeStyle) {
			std::string color;
			value >> color;
			GA.fillColor(v) = color;
		}
		break;
	case graphml::a_nodeStroke:
		if(attrs & GraphAttributes::nodeStyle) {
			std::string color;
			value >> color;
			GA.strokeColor(v) = color;
		}
		break;
	case graphml::a_nodeType:
		if(attrs & GraphAttributes::nodeType) {
			std::string type;
			value >> type;
			GA.type(v) = graphml::toNodeType(type);
		}
		break;
	case graphml::a_template:
		if(attrs & GraphAttributes::nodeTemplate) {
			value >> GA.templateNode(v);
		}
		break;
	case graphml::a_nodeWeight:
		if(attrs & GraphAttributes::nodeWeight) {
			value >> GA.weight(v);
		}
		break;
	default:
		OGDF_WARNING("Unknown attribute with id \""
		     << keyId->getValue()
		     << "\" for node (line "
		     << nodeData.getLine()
		     << "), ignoring.");
	}

	return true;
}


bool GraphMLParser::readData(
	GraphAttributes &GA,
	const edge &e, const XmlTagObject &edgeData)
{
	XmlAttributeObject *keyId;
	edgeData.findXmlAttributeObjectByName("key", keyId);

	if (keyId == nullptr) {
		OGDF_ERROR("Edge data does not have a key.");
		return false;
	}

	const long attrs = GA.attributes();
	std::stringstream value(edgeData.getValue());

	switch(graphml::toAttribute(m_attrName[keyId->getValue()])) {
	case graphml::a_edgeLabel:
		if(attrs & GraphAttributes::edgeLabel) {
			value >> GA.label(e);
		}
		break;
	case graphml::a_edgeWeight:
		if(attrs & GraphAttributes::edgeIntWeight) {
			value >> GA.intWeight(e);
		} else if(attrs & GraphAttributes::edgeDoubleWeight) {
			value >> GA.doubleWeight(e);
		}
		break;
	case graphml::a_edgeType:
		if(attrs & GraphAttributes::edgeType) {
			std::string str;
			value >> str;
			GA.type(e) = graphml::toEdgeType(str);
		}
		break;
	case graphml::a_edgeArrow:
		if(attrs & GraphAttributes::edgeArrow) {
			std::string str;
			value >> str;
			GA.arrowType(e) = graphml::toArrow(str);
		}
		break;
	case graphml::a_edgeStroke:
		if(attrs & GraphAttributes::edgeStyle) {
			std::string color;
			value >> color;
			GA.strokeColor(e) = color;
		}
		break;
	default:
		OGDF_WARNING("Unknown attribute with id \""
		     << keyId->getValue()
		     << "\" for edge (line "
		     << edgeData.getLine()
		     << "), ignoring.");
	}

	return true;
}


bool GraphMLParser::readData(
	ClusterGraphAttributes &CA,
	const cluster &c, const XmlTagObject &clusterData)
{
	XmlAttributeObject *keyId;
	clusterData.findXmlAttributeObjectByName("key", keyId);

	if (keyId == nullptr) {
		OGDF_ERROR("Cluster data does not have a key.");
		return false;
	}

	std::stringstream value(clusterData.getValue());

	using namespace graphml;
	switch(toAttribute(m_attrName[keyId->getValue()])) {
	case a_nodeLabel:
		value >> CA.label(c);
		break;
	case a_x:
		value >> CA.x(c);
		break;
	case a_y:
		value >> CA.y(c);
		break;
	case a_width:
		value >> CA.width(c);
		break;
	case a_height:
		value >> CA.height(c);
		break;
	case a_size:
		double size;
		value >> size;
		// We want to set a new size only if width and height was not set.
		if (CA.width(c) == CA.height(c)) {
			CA.width(c) = CA.height(c) = size;
		}
	case a_r:
		int r;
		value >> r;
		CA.fillColor(c).red(static_cast<uint8_t>(r));
		break;
	case a_g:
		int g;
		value >> g;
		CA.fillColor(c).green(static_cast<uint8_t>(g));
		break;
	case a_b:
		int b;
		value >> b;
		CA.fillColor(c).blue(static_cast<uint8_t>(b));
		break;
	case a_clusterStroke:
		CA.strokeColor(c) = clusterData.getValue();
		break;
	default:
		OGDF_WARNING("Unknown attribute with id \""
		     << keyId->getValue() << "--enum: " << m_attrName[keyId->getValue()] << "--"
		     << "\" for cluster (line "
		     << clusterData.getLine()
		     << "), ignoring.");
	}

	return true;
}


bool GraphMLParser::readNodes(
	Graph &G, GraphAttributes *GA,
	const XmlTagObject &rootTag)
{
	List<XmlTagObject *> nodeTags;
	rootTag.findSonXmlTagObjectByName("node", nodeTags);

	for(XmlTagObject *obj : nodeTags) {
		const XmlTagObject &nodeTag = *obj;

		XmlAttributeObject *idAttr;
		nodeTag.findXmlAttributeObjectByName("id", idAttr);

		if (idAttr == nullptr) {
			OGDF_ERROR("Node is missing id attribute.");
			return false;
		}

		const node v = G.newNode();
		m_nodeId[idAttr->getValue()] = v;

		// Search for data-key attributes if GA given.
		if(GA && !readAttributes(*GA, v, nodeTag)) {
			return false;
		}
	}

	return true;
}


bool GraphMLParser::readEdges(
	Graph &G, GraphAttributes *GA,
	const XmlTagObject &rootTag)
{
	List<XmlTagObject *> edgeTags;
	rootTag.findSonXmlTagObjectByName("edge", edgeTags);

	for(XmlTagObject *obj : edgeTags) {
		const XmlTagObject &edgeTag = *obj;

		XmlAttributeObject *sourceId, *targetId;
		edgeTag.findXmlAttributeObjectByName("source", sourceId);
		edgeTag.findXmlAttributeObjectByName("target", targetId);

		if (sourceId == nullptr) {
			OGDF_ERROR("Edge is missing source node.");
			return false;
		}
		if (targetId == nullptr) {
			OGDF_ERROR("Edge is missing target node.");
			return false;
		}

		const node source = m_nodeId[sourceId->getValue()];
		const node target = m_nodeId[targetId->getValue()];
		const edge e = G.newEdge(source, target);

		// Search for data-key attributes if GA given, return false on error.
		if(GA && !readAttributes(*GA, e, edgeTag)) {
			return false;
		}
	}

	return true;
}


bool GraphMLParser::readClusters(
	Graph &G, ClusterGraph &C, ClusterGraphAttributes *CA,
	const cluster &rootCluster, const XmlTagObject &rootTag)
{
	List<XmlTagObject *> nodeTags;
	rootTag.findSonXmlTagObjectByName("node", nodeTags);

	for(XmlTagObject *obj : nodeTags) {
		const XmlTagObject &nodeTag = *obj;

		XmlAttributeObject *idAttr;
		nodeTag.findXmlAttributeObjectByName("id", idAttr);
		XmlTagObject *clusterTag;
		nodeTag.findSonXmlTagObjectByName("graph", clusterTag);

		if (clusterTag == nullptr) {
			// Got normal node then, add it to the graph - id is required.
			if (idAttr == nullptr) {
				OGDF_ERROR("Node is missing id attribute.");
				return false;
			}

			const node v = G.newNode();
			m_nodeId[idAttr->getValue()] = v;
			C.reassignNode(v, rootCluster);

			// Read attributes when CA given and return false if error.
			if(CA && !readAttributes(*CA, v, nodeTag)) {
				return false;
			}
		} else {
			// Got a cluster node - read it recursively.
			const cluster c = C.newCluster(rootCluster);
			if (!readClusters(G, C, CA, c, *clusterTag)) {
				return false;
			}

			// Read attributes when CA given and return false if error.
			if(CA && !readAttributes(*CA, c, nodeTag)) {
				return false;
			}
		}
	}

	return readEdges(G, CA, rootTag);
}


bool GraphMLParser::read(Graph &G)
{
	// Check whether graph is directed or not (directed by default).
	// XmlAttributeObject *edgeDefaultAttr;
	// m_graphTag->findXmlAttributeObjectByName("edgedefault", edgeDefaultAttr);

	// bool directed = edgeDefaultAttr == nullptr ||
	//                 edgeDefaultAttr->getValue() == "directed";
	if(m_error) {
		return false;
	}

	G.clear();
	m_nodeId.clear();

	return readNodes(G, nullptr, *m_graphTag) && readEdges(G, nullptr, *m_graphTag);
}


bool GraphMLParser::read(Graph &G, GraphAttributes &GA)
{
	if(m_error) {
		return false;
	}

	G.clear();
	m_nodeId.clear();

	return readNodes(G, &GA, *m_graphTag) && readEdges(G, &GA, *m_graphTag);
}


bool GraphMLParser::read(Graph &G, ClusterGraph &C)
{
	if(m_error) {
		return false;
	}

	G.clear();
	m_nodeId.clear();

	return readClusters(G, C, nullptr, C.rootCluster(), *m_graphTag);
}


bool GraphMLParser::read(Graph &G, ClusterGraph &C, ClusterGraphAttributes &CA)
{
	if(m_error) {
		return false;
	}

	G.clear();
	m_nodeId.clear();

	return readClusters(G, C, &CA, C.rootCluster(), *m_graphTag);
}


} // end namespace ogdf
