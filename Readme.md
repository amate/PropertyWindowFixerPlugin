# 設定ダイアログ画面サイズ固定化プラグイン

![](https://raw.githubusercontent.com/amate/PropertyWindowFixerPlugin/images/images/PropertyWindowFixerPlugin_sample1.jpg)
 
## はじめに
このプラグインは、拡張編集の設定ダイアログのウィンドウサイズが長くなったり短くなったりするのを阻止し  
常に同じウィンドウサイズに固定化するプラグインである

## 動作環境
- Windows 10 home 64bit バージョン 1903
- AviUtl 1.00 or 1.10
- 拡張編集 0.92

上記の環境で動作を確認しています  
XPではたぶん動きません(コンパイラが対応していないため)

## 導入方法
aviutl.exeがあるフォルダ、もしくは aviutl.exeがあるフォルダ\plugins\
に PropertyWindowFixerPlugin.auf をコピーしてください

その後、aviutlのメインメニューから、[表示]->[設定ダイアログ画面サイズ固定化プラグイン]をクリックすればウィンドウが表示されます


## アンインストールの方法
PropertyWindowFixerPlugin.auf と PropertyWindowFixerPluginConfig.ini を削除してください

## 設定
GUIは用意していないので、PropertyWindowFixerPluginConfig.ini を直接メモ帳などで編集してください

<pre>
[Config]
cyScrollLine=60
;ホイールスクロール時の移動幅を指定します

cyMargin=120
;設定ダイアログの画面下余白部分を指定します

bAlwaysRestoreScrollPos=false
;v2.0ではv1.2と違い、実装上の理由でタイムラインのアイテム切り替え時に設定ダイアログのスクロール位置が復元されません
;それを強制的にスクロール位置を復元させる設定です (falseをtrueに書き換えてください)
;trueにした場合、切り替え時に少し重くなります
</pre>

## 既知の不具合
設定ダイアログ自身にスクロールバーを付けた影響で少し不具合があります  
- スクロールすると上部中央のシークバーの描画がおかしくなる
- スクロールしてシークバーが表示されていなくても、設定ダイアログ上部中央あたりにシークバーの判定が残っている  
これが実際に問題になることは少ないとは思いますが…

## 免責
作者(原著者＆改変者)は、このソフトによって生じた如何なる損害にも、  
修正や更新も、責任を負わないこととします。  
使用にあたっては、自己責任でお願いします。  
 
何かあれば下記のURLにあるメールフォームにお願いします。  
https://ws.formzu.net/fgen/S37403840/
 
## 著作権表示
Copyright (C) 2019-2020 amate

私が書いた部分のソースコードは、MIT License とします。

## ビルドについて
Visual Studio 2019 が必要です  
ビルドには boost(1.70~)とWTL(10_9163) が必要なのでそれぞれ用意してください。

Boost::Logを使用しているので、事前にライブラリのビルドが必要になります

Boostライブラリのビルド方法
https://boostjp.github.io/howtobuild.html

  //コマンドライン  
  b2.exe install -j 16 --prefix=lib toolset=msvc-14.2  runtime-link=static --with-log --with-filesystem

◆boost  
http://www.boost.org/

◆WTL  
http://sourceforge.net/projects/wtl/

## 更新履歴
<pre>

v2.2
・[fix] オブジェクト切り替え後、設定ダイアログをスクロールしてから設定ダイアログをアクティブにすると、若干スクロールされるのを修正

v2.1
・[fix]設定ダイアログの右上の[+]ボタンを押したときにメニューの表示位置がずれるのを修正

v2.0
・設定ダイアログをスクロールバーを付けた独自フレームウィンドウの子ウィンドウにする実装ではなく、設定ダイアログ自身にスクロールバーを付ける実装へ変更
・[add]設定ダイアログ上でホイールスクロールした時、エディットボックス、トラックバー、コンボボックスにスクロールを吸われないようにした

v1.2
・[fix]設定ダイアログ画面サイズ固定化プラグインを非表示にしたときに、設定ダイアログの大きさがリストアされないのを修正
・[add]PropertyWindowFixerPlugin.aufと同じフォルダにPropertyWindowFixerPluginDebugファイルが存在する時にデバッグ動作させるようにした。
(短時間切り替えで表示アイテムが消える問題が、こちらの環境では再現しないので効果があるかどうかは分からない…)

v1.1
・[change]拡張編集側でのプロパティウィンドウのサイズ制限を抑止するために、WM_WINDOWPOSCHANGINGメッセージを横取りするようにした

v1.0
・完成
</pre>