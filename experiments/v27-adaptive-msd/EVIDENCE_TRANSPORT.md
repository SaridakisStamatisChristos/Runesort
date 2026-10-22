# Evidence byte preservation

The qualification ZIPs contain CRLF CSV summaries. Archive commit 1bb9b408 accidentally normalized those six summary files to LF during text upload. This transport error does not change any values, raw timing CSV bytes, raw SHA-256 values, or gate outcomes. This follow-up restores the original summary bytes, preserving the independently checked byte-identical reproduction contract. Raw inputs, algorithm headers, comparator pins, seeds and thresholds are unchanged.
