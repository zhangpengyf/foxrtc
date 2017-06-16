package org.robolectric.res;

import org.jetbrains.annotations.NotNull;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import java.io.File;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.util.LinkedList;

import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpression;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

public abstract class XpathResourceXmlLoader extends XmlLoader {
  private final String expression;

  public XpathResourceXmlLoader(String expression) {
    this.expression = expression;
  }

  @Override protected void processResourceXml(FsFile xmlFile, XmlNode xmlNode, XmlContext xmlContext) throws Exception {
    for (XmlNode node : xmlNode.selectByXpath(expression)) {
      String name = node.getAttrValue("name");
      processNode(name, node, xmlContext);
    }
  }

  protected abstract void processNode(String name, XmlNode xmlNode, XmlContext xmlContext) throws XPathExpressionException;

  public static class XmlNode {

    private final Node mNode;

    public XmlNode(Node node) {
      mNode = node;
    }

    public String getElementName() {
      if (mNode.getNodeType() == Node.ELEMENT_NODE) {
        return mNode.getNodeName();
      } else {
        return "";
      }
    }

    public XmlNode getFirstChild() {
      NodeList children = mNode.getChildNodes();
      for (int i = 0; i < children.getLength(); i++) {
        if (children.item(i).getNodeType() == Node.ELEMENT_NODE) {
          return new XmlNode(children.item(i));
        }
      }
      return null;
    }

    public String getTextContent() {
      StringBuilder output = new StringBuilder();
      getTextContentHelper(mNode, output);
      return output.toString();
    }

    private void getTextContentHelper(Node node, StringBuilder output) {
      NodeList children = node.getChildNodes();
      if (node.getNodeType() == Node.TEXT_NODE) {
        output.append(node.getNodeValue().trim());
      }
      for (int i = 0; i < children.getLength(); i++) {
        getTextContentHelper(children.item(i), output);
      }
    }

    public Iterable<XmlNode> selectElements(String name) {
      LinkedList<XmlNode> elementsList = new LinkedList();
      Pattern pattern = Pattern.compile(name);
      selectElementsHelper(pattern, mNode, elementsList);
      return elementsList;
    }

    private void selectElementsHelper(Pattern pattern, Node node, LinkedList<XmlNode> list) {
      if (node.getNodeType() == Node.ELEMENT_NODE) {
        Matcher matcher = pattern.matcher(node.getNodeName());
        if (matcher.matches()) {
          list.add(new XmlNode(node));
        }
        NodeList children = node.getChildNodes();
        for (int i = 0; i < children.getLength(); i++) {
          selectElementsHelper(pattern, children.item(i), list);
        }
      }
    }

    public Iterable<XmlNode> selectByXpath(String expression) {
      LinkedList<XmlNode> elementsList = new LinkedList();

      XPathFactory xpathFactory = XPathFactory.newInstance();
      XPath xpath = xpathFactory.newXPath();

      NodeList matchingNodes = null;
      try {
        XPathExpression xpathExpression = xpath.compile(expression);
        matchingNodes = (NodeList) xpathExpression.evaluate(mNode, XPathConstants.NODESET);
      } catch (XPathExpressionException e) {
        return elementsList;
      }
      for (int i = 0; i < matchingNodes.getLength(); i++) {
        if (matchingNodes.item(i).getNodeType() == Node.ELEMENT_NODE) {
          elementsList.add(new XmlNode(matchingNodes.item(i)));
        }
      }
      return elementsList;
    }

    public String getAttrValue(String attributeName) {
      NamedNodeMap attributes = mNode.getAttributes();
      if (attributes != null) {
        Node attribute = attributes.getNamedItem(attributeName);
        if (attribute != null) {
          return attribute.getNodeValue();
        }
      }
      return null;
    }
  }
}
