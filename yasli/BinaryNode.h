/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

namespace yasli{

enum BinaryNode {
  BINARY_NODE_BOOL,
  BINARY_NODE_STRING,
  BINARY_NODE_WSTRING,
  BINARY_NODE_BYTE,
  BINARY_NODE_SBYTE,
  BINARY_NODE_INT16,
  BINARY_NODE_UINT16,
  BINARY_NODE_INT32,
  BINARY_NODE_UINT32,
  BINARY_NODE_INT64,
  BINARY_NODE_UINT64,
  BINARY_NODE_FLOAT,
  BINARY_NODE_DOUBLE,
  BINARY_NODE_POINTER = 0xfd,
  BINARY_NODE_CONTAINER = 0xfe,
  BINARY_NODE_STRUCT = 0xff
};

}
