#pragma once

#include <queue>
#include <future>
#include <thread>

namespace util {

	namespace thread {

		template<class T>
		class ScopeCounter {
		public:
			explicit ScopeCounter(T &v) noexcept
				: _v(v) {
				++_v;
			}

			~ScopeCounter() {
				--_v;
			}

		private:
			T & _v;
		};

		class ThreadPool {
		public:
			explicit ThreadPool(size_t size) noexcept
				: _size(size), _started(false), _stopped(false), _threads(size, nullptr) {}

			virtual ~ThreadPool() {
				Stop();
			}

			ThreadPool &operator=(ThreadPool &&) = default;
			ThreadPool &operator=(const ThreadPool &) = default;

			void Start() noexcept {
				if (Started()) {
					return;
				}

				AsyncStart();
				Wait();
			}

			void AsyncStart() noexcept {
				if (Started()) {
					return;
				}

				for (size_t i = 0; i < _size; i++) {
					_threads[i] = std::make_shared<std::thread>(&ThreadPool::Worker, this, i);
				}

				_started = true;
			}

			void Stop() noexcept {
				AsyncStop();
				Wait();
			}


			void AsyncStop() noexcept {
				if (!Started() || Stopped()) {
					return;
				}

				_stopped = true;
				_cv.notify_all();
			}

			void Wait() noexcept {
				for (auto const &i : _threads) {
					if (i && i->joinable()) {
						i->join();
					}
				}
			}

			bool Started() const noexcept {
				return _started.load();
			}

			bool Stopped() const noexcept {
				return _stopped.load();
			}

			std::exception_ptr Exception() const noexcept {
				return _eptr;
			}

			template<class F, class ...Args>
			auto Commit(F &&f, Args &&...args) -> std::shared_future<decltype(std::bind(f, args...)())> {
				using RT = decltype(std::bind(f, args...)());
				auto fn = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
				auto task = std::make_shared<std::packaged_task<RT()>>(fn);
				std::shared_future<RT> future(task->get_future());
				{
					std::lock_guard<std::mutex> lg(_mutex);
					_tasks.push([task] {
						(*task)();
					});
				}

				_cv.notify_one();
				return future;
			}

		private:
			size_t _size;
			std::mutex _mutex;
			std::atomic_bool _started;
			std::atomic_bool _stopped;
			std::condition_variable _cv;

			using Task = std::function<void()>;
			std::queue<Task> _tasks;

			std::vector<std::shared_ptr<std::thread>> _threads;

			std::exception_ptr _eptr;

		private:
			void Worker(size_t id) {
				try {
					Task task;
					for (;;) {
						{
							std::unique_lock<std::mutex> ul(_mutex);
							while (!Stopped() && _tasks.empty()) {
								_cv.wait(ul);
							}

							if (Stopped()) {
								return;
							}

							task = std::move(_tasks.front());
							_tasks.pop();
						}

						task();
					}
				}
				catch (...) {
					std::unique_lock<std::mutex> ul(_mutex);
					_eptr = std::current_exception();
					AsyncStop();
				}
			}
		};

	}

}