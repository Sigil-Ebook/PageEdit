#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

from __future__ import unicode_literals, print_function

# Copyright 2015 Kevin B. Hendricks, Stratford, Ontario Canada
# Copyright 2012 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

"""Tests for Gumbo CTypes bindings."""

__author__ = 'jdtang@google.com (Jonathan Tang)'

import io
import unittest
import gumboc


class CtypesTest(unittest.TestCase):
    def testWordParse(self):
        with gumboc.parse('Test') as output:
            doctype_node = output.contents.document.contents
            self.assertEqual(gumboc.NodeType.DOCUMENT, doctype_node.type)
            document = doctype_node.v.document
            self.assertEqual(b'', document.name)
            self.assertEqual(b'', document.public_identifier)
            self.assertEqual(b'', document.system_identifier)

            root = output.contents.root.contents
            self.assertEqual(gumboc.NodeType.ELEMENT, root.type)
            self.assertEqual(gumboc.Tag.HTML, root.tag)
            self.assertEqual(gumboc.Namespace.HTML, root.tag_namespace)
            self.assertEqual(2, len(root.children))

            head = root.children[0]
            self.assertEqual(gumboc.NodeType.ELEMENT, head.type)
            self.assertEqual(gumboc.Tag.HEAD, head.tag)
            self.assertEqual(b'head', head.tag_name)
            self.assertEqual(gumboc.Namespace.HTML, head.tag_namespace)
            self.assertEqual(0, len(head.original_tag))
            self.assertEqual(b'', bytes(head.original_end_tag))
            self.assertEqual(0, head.children.length)

            body = root.children[1]
            self.assertNotEqual(body, doctype_node)
            self.assertEqual(gumboc.NodeType.ELEMENT, body.type)
            self.assertEqual(gumboc.Tag.BODY, body.tag)
            self.assertEqual(b'body', body.tag_name)
            self.assertEqual(1, len(body.children))

            text_node = body.children[0]
            self.assertEqual(gumboc.NodeType.TEXT, text_node.type)
            self.assertEqual(b'Test', text_node.text)

    def testBufferThatGoesAway(self):
        for i in range(10):
            source = io.StringIO('<foo bar=quux>1<p>2</foo>')
            parse_tree = gumboc.parse(source.read())
            source.close()
        with parse_tree as output:
            root = output.contents.root.contents
            body = root.children[1]
            foo = body.children[0]
            self.assertEqual(gumboc.NodeType.ELEMENT, foo.type)
            self.assertEqual(gumboc.Tag.UNKNOWN, foo.tag)
            self.assertEqual(b'<foo bar=quux>', bytes(foo.original_tag))
            self.assertEqual(b'', bytes(foo.original_end_tag))
            self.assertEqual(b'foo', foo.tag_name)
            self.assertEqual(b'bar', foo.attributes[0].name)
            self.assertEqual(b'quux', foo.attributes[0].value)

    def testUnknownTag(self):
        with gumboc.parse('<foo bar=quux>1<p>2</foo>') as output:
            root = output.contents.root.contents
            body = root.children[1]
            foo = body.children[0]
            self.assertEqual(gumboc.NodeType.ELEMENT, foo.type)
            self.assertEqual(gumboc.Tag.UNKNOWN, foo.tag)
            self.assertEqual(b'<foo bar=quux>', bytes(foo.original_tag))
            self.assertEqual(b'', bytes(foo.original_end_tag))
            self.assertEqual(b'foo', foo.tag_name)
            self.assertEqual(b'bar', foo.attributes[0].name)
            self.assertEqual(b'quux', foo.attributes[0].value)

    def testSarcasm(self):
        with gumboc.parse('<div><sarcasm><div></div></sarcasm></div>') as output:
            root = output.contents.root.contents
            body = root.children[1]
            div = body.children[0]
            sarcasm = div.children[0]
            self.assertEqual(gumboc.NodeType.ELEMENT, sarcasm.type)
            self.assertEqual(gumboc.Tag.UNKNOWN, sarcasm.tag)
            self.assertEqual(b'<sarcasm>', bytes(sarcasm.original_tag))
            self.assertEqual(b'</sarcasm>', bytes(sarcasm.original_end_tag))
            self.assertEqual(b'sarcasm', sarcasm.tag_name)

    def testEnums(self):
        self.assertEqual(gumboc.Tag.A, gumboc.Tag.A)
        self.assertEqual(hash(gumboc.Tag.A.value), hash(gumboc.Tag.A))

    def testFragment(self):
        with gumboc.parse(
            '<div></div>',
            container=gumboc.Tag.TITLE,
            container_namespace=gumboc.Namespace.SVG) as output:
            root = output.contents.root.contents
            self.assertEqual(1, len(root.children))
            div = root.children[0]
            self.assertEqual(gumboc.NodeType.ELEMENT, div.type)
            self.assertEqual(gumboc.Tag.DIV, div.tag)
            self.assertEqual(gumboc.Namespace.HTML, div.tag_namespace)

if __name__ == '__main__':
    unittest.main()
