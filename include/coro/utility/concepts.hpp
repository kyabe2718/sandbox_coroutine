#pragma once

#if __has_include(<coroutine>)
#include <coroutine>
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
namespace std {
using namespace std::experimental;
}
#else
#error cannot include a header for coroutine
#endif

#include "type_traits.hpp"

namespace coro {

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
using awaitable_to_awaiter_t = typename awaitable_traits<Awaitable>::awaiter_type;

template<typename A>
concept Awaiter = !std::is_void_v<A> && requires(A a) {

    // trueならコルーチンを中断せずにco_await式が即座に返る
    { a.await_ready() } -> std::same_as<bool>;

    // コルーチンを中断する前の処理 現在のコルーチンに対応するhandle
    // void -> 現在のコルーチンを中断し，その呼び出し元・再開元へ返る
    // bool -> trueならvoidと同じ falseならco_await式が即座に返る
    // coroutine_handle -> 現在のコルーチンを中断し，await_suspendの返り値に対応するコルーチンが再開される
    { a.await_suspend(std::coroutine_handle<>{} /* caller */) } -> convertible_to_any_of<void, bool, std::coroutine_handle<>>;

    // コルーチンが再開された直後の動作
    // この返り値がco_await式の返り値になる
    a.await_resume();
};

template<typename A>
concept Awaitable = Awaiter<typename awaitable_traits<A>::awaiter_type>;

static_assert(Awaitable<std::suspend_never>);
static_assert(Awaitable<std::suspend_always>);
}// namespace coro
