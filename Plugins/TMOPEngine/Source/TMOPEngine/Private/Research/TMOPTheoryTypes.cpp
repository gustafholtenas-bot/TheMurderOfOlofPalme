#include "Research/TMOPTheoryTypes.h"

TArray<FTMOPTheoryInformationRow> TMOPTheory::DefaultInformation()
{
    // Headings supplied in the design references, not verified source text.
    // Empty bodies are explicitly shown as not yet authored in the UI.
    TArray<FTMOPTheoryInformationRow> Rows;
    auto Add = [&Rows](ETMOPTheoryInformationTrack Track, const TCHAR* Title)
    {
        FTMOPTheoryInformationRow Row; Row.Track = Track; Row.SortOrder = Rows.Num();
        Row.Title = FText::FromString(Title); Rows.Add(Row);
    };
    Add(ETMOPTheoryInformationTrack::LoneGunman, TEXT("Erkännandet från B"));
    Add(ETMOPTheoryInformationTrack::LoneGunman, TEXT("Erkännandet från A"));
    Add(ETMOPTheoryInformationTrack::LoneGunman, TEXT("Uppgiften om att Pettersson tog fel person"));
    Add(ETMOPTheoryInformationTrack::LoneGunman, TEXT("Aktieägares hämnd"));
    Add(ETMOPTheoryInformationTrack::Conspiracy, TEXT("Erkännandet från X"));
    Add(ETMOPTheoryInformationTrack::Conspiracy, TEXT("Erkännandet från Y"));
    Add(ETMOPTheoryInformationTrack::Conspiracy, TEXT("Brevet tillskrivet Victor Gunnarsson"));
    Add(ETMOPTheoryInformationTrack::Conspiracy, TEXT("Den sydafrikanska agentens uppgifter"));
    Add(ETMOPTheoryInformationTrack::Conspiracy, TEXT("Bertil Wedins uppgifter"));
    return Rows;
}

FString TMOPTheory::TemplateName(int32 Index)
{
    switch (Index)
    {
    case 0: return TEXT("Ensam gärningsman");
    case 1: return TEXT("Liten konspiration");
    case 2: return TEXT("Stor konspiration");
    default: return TEXT("Stor konspiration + övervakning");
    }
}

FGuid TMOPTheory::AddNode(FTMOPTheoryTree& Tree, ETMOPTheoryNodeKind Kind,
    const FVector2D& Position, const FString& Title)
{
    FTMOPTheoryNode Node;
    Node.Id = FGuid::NewGuid();
    Node.Kind = Kind;
    Node.Position = Position;
    Node.Title = Title;
    Node.Role = Kind == ETMOPTheoryNodeKind::Person ? TEXT("Person") : Kind == ETMOPTheoryNodeKind::Vehicle ? TEXT("Fordon") : TEXT("Anteckning");
    Node.Size = Kind == ETMOPTheoryNodeKind::Person ? FVector2D(80, 104) :
        Kind == ETMOPTheoryNodeKind::Vehicle ? FVector2D(154, 72) : FVector2D(240, 110);
    Tree.Nodes.Add(Node);
    return Node.Id;
}

bool TMOPTheory::AddLink(FTMOPTheoryTree& Tree, FGuid From, FGuid To)
{
    if (From == To || !Tree.Nodes.ContainsByPredicate([From](const auto& N) { return N.Id == From; }) ||
        !Tree.Nodes.ContainsByPredicate([To](const auto& N) { return N.Id == To; }) ||
        Tree.Links.ContainsByPredicate([From, To](const auto& L)
        { return (L.From == From && L.To == To) || (L.From == To && L.To == From); })) return false;
    FTMOPTheoryLink Link;
    Link.Id = FGuid::NewGuid();
    Link.From = From;
    Link.To = To;
    Tree.Links.Add(Link);
    return true;
}

bool TMOPTheory::RemoveNode(FTMOPTheoryTree& Tree, FGuid Id)
{
    const FTMOPTheoryNode* Node = Tree.Nodes.FindByPredicate([Id](const auto& N) { return N.Id == Id; });
    if (!Node || Node->bShooter) return false;
    Tree.Links.RemoveAll([Id](const auto& L) { return L.From == Id || L.To == Id; });
    Tree.Nodes.RemoveAll([Id](const auto& N) { return N.Id == Id; });
    return true;
}

bool TMOPTheory::AssignObservation(FTMOPTheoryTree& Tree, FGuid NodeId,
    FName EntityId, const TArray<FTMOPNotebookObservation>& Observations)
{
    FTMOPTheoryNode* Node = Tree.Nodes.FindByPredicate([NodeId](const auto& N) { return N.Id == NodeId; });
    if (!Node || Node->bShooter || Node->Kind == ETMOPTheoryNodeKind::Note) return false;
    const auto Kind = Node->Kind == ETMOPTheoryNodeKind::Vehicle
        ? ETMOPNotebookEntityKind::Vehicle : ETMOPNotebookEntityKind::Person;
    const auto* Entry = Observations.FindByPredicate([EntityId, Kind](const auto& O)
        { return O.EntityId == EntityId && O.Kind == Kind; });
    if (!Entry || (Kind == ETMOPNotebookEntityKind::Person && Entry->Category == ETMOPNotebookCategory::Shooter)) return false;
    if (Tree.Nodes.ContainsByPredicate([EntityId, Node, NodeId](const auto& N)
        { return N.Id != NodeId && N.Kind == Node->Kind && N.EntityId == EntityId; })) return false;
    Node->EntityId = Entry->EntityId;
    Node->Title = Entry->DisplayName.ToString();
    return true;
}

void TMOPTheory::SynchronizeShooter(FTMOPTheoryTree& Tree,
    const TArray<FTMOPNotebookObservation>& Observations)
{
    const auto* Shooter = Observations.FindByPredicate([](const auto& O)
        { return O.Kind == ETMOPNotebookEntityKind::Person && O.Category == ETMOPNotebookCategory::Shooter; });
    for (auto& Node : Tree.Nodes)
    {
        if (!Node.bShooter) continue;
        Node.Kind = ETMOPTheoryNodeKind::Person;
        Node.EntityId = Shooter ? Shooter->EntityId : NAME_None;
        Node.Title = Shooter ? Shooter->DisplayName.ToString() : TEXT("Skytten — ännu inte hittad");
    }
}

FTMOPTheoryTree TMOPTheory::CreateTemplate(int32 Index)
{
    Index = FMath::Clamp(Index, 0, 3);
    FTMOPTheoryTree Tree;
    Tree.Id = FGuid::NewGuid();
    Tree.TemplateIndex = FMath::Clamp(Index, 0, 3);
    Tree.Title = TemplateName(Index);
    // Coordinates follow the four supplied mockups; no inferred connections are
    // prefilled. Players draw their own relationships.
    auto Slot = [&Tree](double X, double Y, const TCHAR* Role, bool bVehicle = false, bool bShooter = false)
    {
        AddNode(Tree, bVehicle ? ETMOPTheoryNodeKind::Vehicle : ETMOPTheoryNodeKind::Person,
            FVector2D((X - 350) * 1.4, Y * 1.4), TEXT(""));
        auto& N = Tree.Nodes.Last(); N.Role = Role; N.bShooter = bShooter;
        if (bShooter) N.Title = TEXT("Ännu inte hittad");
    };
    auto Heading = [&Tree](double X, double Y, const TCHAR* Text)
    {
        AddNode(Tree, ETMOPTheoryNodeKind::Note, FVector2D((X - 350) * 1.4, Y * 1.4), Text);
        auto& N = Tree.Nodes.Last(); N.bHeading = true; N.Size = FVector2D(360, 52); N.Role.Empty();
    };
    if (Index == 0)
    {
        Slot(1035, 428, TEXT("Gärningsmannen"), false, true);
        return Tree;
    }
    const double RowY = Index == 1 ? 557 : 620;
    Heading(1035, Index == 1 ? 385 : 448, TEXT("Mördargrupp"));
    Slot(1045, Index == 1 ? 439 : 502, TEXT("Ledare"));
    Slot(934, RowY, TEXT("Kumpan")); Slot(1006, RowY, TEXT("Kumpan")); Slot(1088, RowY, TEXT("Kumpan"));
    Slot(1192, RowY, TEXT("Gärningsmannen"), false, true);
    if (Index == 1)
    {
        Slot(868, 680, TEXT("Fordon"), true); Slot(1175, 680, TEXT("Fordon"), true);
        return Tree;
    }
    Slot(1046, 181, TEXT("Uppdragsgivare"));
    Slot(1052, 343, TEXT("Mellanhand"));
    Heading(1490, 70, TEXT("Personer där men ej involverade / möjliga syndabockar"));
    for (double X : {1515.0, 1594.0, 1673.0}) Slot(X, 133, TEXT(""));
    Heading(1550, 535, TEXT("Hjälpgrupp"));
    for (double X : {1432.0, 1523.0, 1605.0, 1684.0}) Slot(X, 620, TEXT("Spanare"));
    for (double X : {879.0, 1003.0, 1127.0, 1442.0, 1613.0}) Slot(X, 743, TEXT("Fordon"), true);
    if (Index == 3)
    {
        Heading(590, 390, TEXT("SÄPO"));
        Slot(591, 456, TEXT("Ledare"));
        for (double X : {459.0, 535.0, 611.0, 689.0}) Slot(X, 611, TEXT("Spanare"));
        Slot(482, 744, TEXT("Fordon"), true); Slot(618, 744, TEXT("Fordon"), true);
    }
    return Tree;
}
