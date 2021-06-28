#pragma once

#include <concepts>

#if __has_include(<coroutine>)
#include <coroutine>
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
namespace std {
using namespace std::experimental;
}
#else
#error cannot include the header for coroutine
#endif


namespace coro {

template<typename T, typename... Ts>
concept same_as_any_of = (std::same_as<T, Ts> || ...);

template<typename T, typename... Ts>
concept convertible_to_any_of = (std::is_convertible_v<T, Ts> || ...);

namespace detail {
template<typename P>
concept found_return_void = requires(P p) {
    p.return_void();
};

template<typename P>
concept found_return_value = requires(P p) {
    typename P::value_type;
    p.return_value(std::declval<typename P::value_type>());
};

template<typename Awaitable>
concept member_co_await = requires(Awaitable a) { a.operator co_await(); };

template<typename Awaitable>
concept non_member_co_await = requires(Awaitable a) { operator co_await(static_cast<Awaitable &&>(a)); };

template<typename Awaitable>
struct awaitable_to_awaiter { using type = Awaitable; };

template<member_co_await Awaitable>
struct awaitable_to_awaiter<Awaitable> { using type = decltype(std::declval<Awaitable>().operator co_await()); };

template<non_member_co_await Awaitable>
struct awaitable_to_awaiter<Awaitable> { using type = decltype(operator co_await(std::declval<Awaitable>())); };
}// namespace detail

template<typename P>
concept Promise = requires(P p) {
    // Promiseオブジェクトをデフォルト構築する
    requires std::is_default_constructible_v<P>;

    // コルーチンオブジェクトを取得
    p.get_return_object();

    // 初期サスペンドポイント
    /*co_await*/ p.initial_suspend();

    // co_return v; -> p.return_value(v);
    // co_return; -> p.return_void();
    // 戻り値をpromiseオブジェクト経由で持ち出す
    requires(!detail::found_return_value<P> && detail::found_return_void<P>) || (detail::found_return_value<P> && !detail::found_return_void<P>);

    // 例外が投げられていれば，それを処理する必要がある
    { p.unhandled_exception() } -> std::same_as<void>;

    // 最終サスペンドポイント
    // ここで中断されている時のみ，coroutine_handle::done()はtrueを返す
    /*co_await*/ p.final_suspend();

    requires !std::is_copy_constructible_v<P>;
    requires !std::is_copy_assignable_v<P>;
};

// co_await expr; とすると，exprは一定の規則に従ってawaitableに変換される
// yield式，初期サスペンドポイント，最終サスペンドポイントならexprそのまま
// p.await_transform(expr) が有効ならその値
// そうでなければexprそのまま

// awaitableは，operator co_await (メンバ関数or非メンバ関数)によってawaiterに変換される
// 適合するoperator co_awaitが見つからなければ，awaitableはそのままawaiterとして扱われる
template<typename Awaitable>
using awaitable_to_awaiter_t = typename detail::awaitable_to_awaiter<Awaitable>::type;

template<typename A>
concept Awaiter = requires(A a) {

    // trueならコルーチンを中断せずにco_await式が即座に返る
    { a.await_ready() } -> std::same_as<bool>;

    // コルーチンを中断する前の処理 現在のコルーチンに対応するhandle
    // void -> 現在のコルーチンを中断し，その呼び出し元へ返る
    // bool -> trueならvoidと同じ falseならco_await式が即座に返る
    // coroutine_handle -> 現在のコルーチンを中断し，await_suspendの返り値に対応するコルーチンが再開される
    { a.await_suspend(std::coroutine_handle<>{} /* caller */) } -> convertible_to_any_of<void, bool, std::coroutine_handle<>>;

    // コルーチンが再開された直後の動作
    // この返り値がco_await式の返り値になる
    a.await_resume();
};

template<typename A>
concept Awaitable = Awaiter<awaitable_to_awaiter_t<A>>;

static_assert(Awaitable<std::suspend_never>);
static_assert(Awaitable<std::suspend_always>);
}// namespace coro
