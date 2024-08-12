// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <boost/filesystem.hpp>
#include "tx_header.h"

TX_NAMESPACE_OPEN(Utils)

using FilePath = boost::filesystem::path;

/**
 * @brief 创建目录
 *
 * @param strDirPath 路径
 * @return Base::txBool
 */
Base::txBool CreateDirectory(Base::txString const& strDirPath) TX_NOEXCEPT;

/**
 * @brief 获取文件名拓展后缀
 *
 * @param strFilePath 文件路径
 * @param[out] refStrExternsion 后缀
 * @return Base::txBool
 */
Base::txBool GetFileExtension(Base::txString const& strFilePath, Base::txString& refStrExternsion) TX_NOEXCEPT;

/**
 * @brief 获取app path
 *
 * @param str_argv
 * @param refAppPath
 * @return Base::txBool
 */
Base::txBool GetAppPath(Base::txString str_argv, const char* refAppPath) TX_NOEXCEPT;

/**
 * @brief 获取app path
 *
 * @param str_argv
 * @param refAppPath
 * @return Base::txBool
 */
Base::txBool GetAppPath(Base::txString str_argv, Base::txString& refAppPath) TX_NOEXCEPT;
/*
std::cout <<"                           root_path:    "<< path.root_path().string() << std::endl;
std::cout << "                          root_name:    " << path.root_name().string() << std::endl ;
std::cout << "                     root_directory:    " << path.root_directory().string() << std::endl << std::endl;
std::cout << "                      relative_path:    " << path.relative_path().string() << std::endl ;
std::cout << "      relative_path.relative_path():    " << path.relative_path().relative_path().string() << std::endl;;
std::cout << "               relative_path.stem():    " << path.relative_path().stem() << std::endl << std::endl;;
std::cout << "                        parent_path:    " << path.parent_path().string() << std::endl ;
std::cout << "                   parent_path.stem:    " << path.parent_path().stem().string() << std::endl ;;
std::cout << "          parent_path.parent_path():    " << path.parent_path().parent_path().string() << std::endl ;
std::cout << ".parent_path().parent_path().stem():    " << path.parent_path().parent_path().stem().string() << std::endl
<< std::endl; std::cout << "                         filename():    " << path.filename().string() << std::endl;
std::cout << "                  filename().stem():    " << path.filename().stem().string() << std::endl << std::endl;
std::cout << "                    has_extension():    " << path.has_extension()<< std::endl;
std::cout << "                        extension():    " << path.extension() << std::endl ;
std::cout << "                  replace_extension:    " << path.replace_extension(".las") << std::endl << std::endl;
std::cout << "                       is_absolute:    " << path.is_absolute() << std::endl;
std::cout << "          relative_path.is_relative:    " << path.relative_path().is_relative() << std::endl;
*/

TX_NAMESPACE_CLOSE(Utils)
