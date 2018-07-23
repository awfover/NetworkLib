#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include <Network/Protocol/Tcp.h>

#include <Network/Ftp/Cmd.h>
#include <Network/Ftp/Type.h>
#include <Network/Ftp/Reply.h>
#include <Network/Ftp/Const.h>

#include <Network/Ftp/Parser/HostPortParser.h>
#include <Network/Ftp/Parser/FileListParser.h>

#include <network/Parser/DQuotedParser.h>

#include <Util/IO.h>
#include <Util/Error.h>
#include <Util/Buffer.h>

namespace network {

	namespace ftp {

		class Client : public Tcp::Socket {
		public:
			using DynamicStringBuffer = util::buffer::DynamicStringBuffer<char, std::char_traits<char>, std::allocator<char>>;

		public:
			Client(util::io::IOContext &ctx, 
				const Tcp &protocol,
				const Tcp::Resolver::Result::Ptr &server_endpoints, 
				const std::string &user = std::string(FTP_ANONYMOUS),
				const std::string &pass = std::string(FTP_ANONYMOUS),
				size_t buffer_size = 65535) noexcept
				: Tcp::Socket(ctx, protocol),
				_server_endpoints(server_endpoints),
				_user(user),
				_pass(pass),
				_buffer(_buff, buffer_size),
				_buffer_size(buffer_size) {}

			void Init(util::error::Error &err) {
				Connect(_server_endpoints, err);
				if (err) {
					return;
				}

				Welcome(err);
				if (err) {
					return;
				}

				Login(err);
				if (err) {
					return;
				}
			}

			void Pwd(std::string &dir, util::error::Error &err) {
				Reply::Sequence rs;
				SendCmd(rs, CmdType::PWD, err);
				if (err) {
					return;
				}

				network::parser::DQuotedParser parser(dir);
				parser.Input(rs.front()->Msg());
				parser.Eoi();
				if (!parser.Succeeded()) {
					err = error::FtpError(error::FtpErrorCode::REPLY_BAD_MSG, "pwd");
					return;
				}
			}

			void Cwd(const std::string &dir, util::error::Error &err) {
				Reply::Sequence rs;
				if (!SendCmd(rs, CmdType::CWD, err, dir)) {
					if (!err) {
						err = error::FtpError(error::FtpErrorCode::CWD_FAILED, "cwd");
					}
					return;
				}
			}

			void List(File::List &list, util::error::Error &err) {
				Tcp::Socket conn(_ctx, _protocol);
				OpenDataConnection(conn, err);
				if (err) {
					return;
				}
				
				auto read_future = _ctx.Commit(&Client::ReadFileList, this, &list, &conn);

				Reply::Sequence rs;
				SendCmd(rs, CmdType::LIST, err);
				if (err) {
					return;
				}

				read_future.wait();
				err = read_future.get();
				if (err) {
					return;
				}

				if (!WaitForReply(err)) {
					return;
				}
			}

			void Download(std::basic_ostream<char, std::char_traits<char>> &os, const std::string &src_path, util::error::Error &err) {
				Tcp::Socket conn(_ctx, _protocol);
				OpenDataConnection(conn, err);
				if (err) {
					return;
				}

				auto read_future = _ctx.Commit(&Client::ReadAll, this, &os, &conn);

				Reply::Sequence rs;
				SendCmd(rs, CmdType::RETR, err, src_path);
				if (err) {
					return;
				}

				read_future.wait();
				err = read_future.get();
				if (err) {
					return;
				}

				if (!WaitForReply(err)) {
					return;
				}
			}

			void Download(const std::string &dst_path, const std::string &src_path, util::error::Error &err) {
				std::ofstream os(dst_path, std::ios::binary);
				if (!os.is_open()) {
					err = util::error::IOError(util::error::IOErrorCode::OPEN_FILE_FAILED, "download");
					return;
				}

				Download(dynamic_cast<std::basic_ostream<char, std::char_traits<char>> &>(os), src_path, err);
				os.close();
			}

			void Upload(std::basic_istream<char, std::char_traits<char>> &is, const std::string &dst_path, util::error::Error &err) {
				Tcp::Socket conn(_ctx, _protocol);
				OpenDataConnection(conn, err);
				if (err) {
					return;
				}

				auto read_future = _ctx.Commit(&Client::WriteAll, this, &is, &conn);

				Reply::Sequence rs;
				SendCmd(rs, CmdType::STOR, err, dst_path);
				if (err) {
					return;
				}

				read_future.wait();
				err = read_future.get();
				if (err) {
					return;
				}

				conn.Close(err);
				if (err) {
					return;
				}

				if (!WaitForReply(err)) {
					return;
				}
			}

			void Upload(const std::string &dst_path, const std::string &src_path, util::error::Error &err) {
				std::ifstream is(src_path, std::ios::binary);
				if (!is.is_open()) {
					err = util::error::IOError(util::error::IOErrorCode::OPEN_FILE_FAILED, "upload");
					return;
				}

				Upload(dynamic_cast<std::basic_istream<char, std::char_traits<char>> &>(is), dst_path, err);
				is.close();
			}

		private:
			Tcp::Resolver::Result::Ptr _server_endpoints;

			std::string _user;
			std::string _pass;
			
			std::string _buff;
			DynamicStringBuffer _buffer;

			size_t _buffer_size;

		private:
			bool SendCmd(Reply::Sequence &rs, const Cmd &c, util::error::Error &err) {
				util::io::Write(*this, util::buffer::ConstBuffer::From(c.Str()), err);
				if (err) {
					return false;
				}

				WaitForReply(rs, err);
				if (err) {
					return false;
				}

				return true;
			}

			bool SendCmd(const Cmd &c, util::error::Error &err) {
				Reply::Sequence rs;
				return SendCmd(rs, c, err);
			}

			template<class ...Args>
			bool SendCmd(Reply::Sequence &rs, CmdType t, util::error::Error &err, Args &&...args) {
				Cmd c(t, std::forward<Args>(args)...);
				return SendCmd(rs, c, err);
			}

			template<class ...Args>
			bool SendCmd(CmdType t, util::error::Error &err, Args &&...args) {
				Reply::Sequence rs;
				Cmd c(t, std::forward<Args>(args)...);
				return SendCmd(rs, c, err);
			}

			void WaitForReply(Reply::Sequence &rs, util::error::Error &err) {
				using namespace std::string_literals;

				size_t nread;
				std::string raw_reply;
				while (!rs.End()) {
					nread = util::io::ReadUntil(*this, _buffer, err, "\r\n"s, "\n"s);
					if (err) {
						return;
					}

					_buffer.Consume(raw_reply, nread);
					rs.Parse(raw_reply, err);
					if (err) {
						return;
					}
				}

				if (!rs.Positive()) {
					err = error::FtpError(error::FtpErrorCode::REPLY_NEGATIVE);
					return;
				}
			}

			bool WaitForReply(util::error::Error &err) {
				Reply::Sequence rs;
				WaitForReply(rs, err);
				if (err) {
					return false;
				}

				return true;
			}

			void Welcome(util::error::Error &err) {
				Reply::Sequence rs;
				WaitForReply(rs, err);
				if (err) {
					return;
				}
			}
				 
			void Login(util::error::Error &err) {
				if (!SendCmd(CmdType::USER, err, _user)) {
					if (!err) {
						err = error::FtpError(error::FtpErrorCode::LOGIN_FAILED);
					}
					return;
				}

				if (!SendCmd(CmdType::PASS, err, _pass)) {
					if (!err) {
						err = error::FtpError(error::FtpErrorCode::LOGIN_FAILED);
					}
					return;
				}
			}

			void Pasv(std::string &host, std::string &port, util::error::Error &err) {
				Reply::Sequence rs;
				if (!SendCmd(rs, CmdType::PASV, err)) {
					return;
				}

				parser::HostPortParser parser(host, port);
				parser.Input(rs.front()->Msg());
				parser.Eoi();
				if (!parser.Succeeded()) {
					err = error::FtpError(error::FtpErrorCode::INVALID_HOST_PORT);
					return;
				}
			}

			void OpenDataConnection(Tcp::Socket &conn, util::error::Error &err) {
				std::string host;
				std::string port;
				Pasv(host, port, err);
				if (err) {
					return;
				}

				conn.Connect(host, port, err);
				if (err) {
					return;
				}
			}

		private:
			util::error::Error ReadFileList(
				std::vector<std::shared_ptr<File>> *list, 
				Tcp::Socket *conn) {
				util::error::Error err;
				util::error::Error r_err = error::FtpError(error::FtpErrorCode::READ_FILE_LIST_FAILED).Error();
				if (!list || !conn) {
					return r_err;
				}

				size_t nread;
				std::string buff(_buffer_size, 0);
				network::ftp::parser::FileListParser parser(*list);
				while (conn->IsOpen()) {
					nread = conn->ReadSome(util::buffer::MutableBuffer::From(buff), err);
					if (err) {
						return err;
					}

					parser.Input(util::buffer::ConstBuffer::From(buff, nread));
					if (parser.Failed()) {
						return err;
					}
				}

				parser.Eoi();
				if (!parser.Succeeded()) {
					return r_err;
				}

				return err;
			}

			util::error::Error ReadAll(
				std::basic_ostream<char, std::char_traits<char>> *os,
				Tcp::Socket *conn) {
				util::error::Error err;
				util::error::Error r_err = error::FtpError(error::FtpErrorCode::READ_DATA_CONN_FAILED).Error();
				if (!os || !conn) {
					return r_err;
				}

				size_t nread;
				std::string buff(_buffer_size, 0);
				while (conn->IsOpen()) {
					nread = conn->ReadSome(util::buffer::MutableBuffer::From(buff), err);
					if (err) {
						return err;
					}

					os->write(buff.c_str(), nread);
				}

				return err;
			}

			util::error::Error WriteAll(
				std::basic_istream<char, std::char_traits<char>> *is,
				Tcp::Socket *conn) {
				util::error::Error err;
				util::error::Error r_err = error::FtpError(error::FtpErrorCode::WRITE_DATA_CONN_FAILED).Error();
				if (!is || !conn) {
					return r_err;
				}

				std::streampos curp = is->tellg();
				is->seekg(0, std::ios::end);
				std::streampos endp = is->tellg();
				is->seekg(curp);

				std::streamsize aread = endp - curp;

				size_t nread = _buffer_size;
				std::string buff(_buffer_size, 0);
				while (conn->IsOpen() && (aread > 0)) {
					if (aread < nread) {
						nread = (size_t)aread;
					}

					is->read(&buff[0], nread);
					util::io::Write(*conn, util::buffer::ConstBuffer::From(buff, nread), err);
					if (err) {
						return err;
					}

					aread -= nread;
				}

				return err;
			}
		};

	}

}