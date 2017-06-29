### MountainCompare

MountainCompare is a GUI tool used in order to compare two different sortings of the same data.

Launch mountaincompare using:

> mountaincompare --firings1=firings1.mda --firings2=firings2.mda --samplerate=30000 --raw=raw.mda --filt=filt.mda --pre=pre.mda

This will bring up different views from [mountainview](mountainview.md).

New in this view is the confusion matrix.

**Confusion Matrix**

A confusion matrix (or contingency table (Zaki and Meira, 2014)) summarizes the consistency between two sortings of the same data by showing the pairwise counts. The entry a<sub>ij</sub> represents the number of events that were classified as *i* in the first sorting and as *j* in the second. To account for the arbitrary permutation of labels, the rows and columns are permuted to maximize the sum of the diagonal entries (Kuhn, 1955). A purely diagonal matrix corresponds to perfect agreement.

In this GUI view, there are multiple ways to permute the labels. Also, both Row-normalized (normalized for sorting 1) and Column-normalized (normalized for sorting 2) are shown.

Clicking on an entry in any of the confusion matrices will highight the two clusters in the corresponding mountainview window.
