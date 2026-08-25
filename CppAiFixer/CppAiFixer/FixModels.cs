using System.Collections.Generic;
using System.Runtime.Serialization;

namespace CppAiFixer
{
    /// <summary>一次精确文本替换。</summary>
    [DataContract]
    public sealed class FixEdit
    {
        [DataMember(Name = "old")] public string Old { get; set; }
        [DataMember(Name = "new")] public string New { get; set; }
    }

    /// <summary>一种候选修复做法。</summary>
    [DataContract]
    public sealed class FixCandidate
    {
        [DataMember(Name = "title")] public string Title { get; set; }
        [DataMember(Name = "explanation")] public string Explanation { get; set; }
        [DataMember(Name = "edits")] public List<FixEdit> Edits { get; set; }
    }

    /// <summary>模型返回的完整响应。</summary>
    [DataContract]
    public sealed class FixResponse
    {
        [DataMember(Name = "candidates")] public List<FixCandidate> Candidates { get; set; }
    }
}
