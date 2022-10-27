# neoSYCL_RS_examples
neoSYCLの依存関係を解決する部分の挙動を確認するためのtestたちです

## Dead_Lock_example
* 従来の実装だと半分くらいデッドロックになります

## relationship
* 足し算をするタスクと掛け算をするタスクを交互に行います。
* 実行順序がバラバラだと実行ごとに出力結果が異なります。

## TimelineWatcher
* 2つのタスクが実行されている時間を確認できます。
* 両方のタスクでbuffer Aを使います。
* buffer Aのaccess modeを変更することで並列に実行されたり直列に実行されたりする様子を確認できます。
